#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")

static std::string read_file(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out.push_back(' ');
        } else if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            char *end = nullptr;
            long v = std::strtol(hex, &end, 16);
            if (end && *end == 0) {
                out.push_back(static_cast<char>(v));
                i += 2;
            } else {
                out.push_back(s[i]);
            }
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::unordered_map<std::string, std::string> parse_form(const std::string &body) {
    std::unordered_map<std::string, std::string> out;
    size_t start = 0;
    while (start < body.size()) {
        size_t amp = body.find('&', start);
        if (amp == std::string::npos) amp = body.size();
        size_t eq = body.find('=', start);
        if (eq != std::string::npos && eq < amp) {
            std::string k = body.substr(start, eq - start);
            std::string v = body.substr(eq + 1, amp - eq - 1);
            out[url_decode(k)] = url_decode(v);
        }
        start = amp + 1;
    }
    return out;
}

static std::string content_type_for(const std::string &path) {
    if (path.size() >= 5 && path.substr(path.size()-5) == ".html") return "text/html; charset=utf-8";
    if (path.size() >= 3 && path.substr(path.size()-3) == ".js") return "application/javascript; charset=utf-8";
    if (path.size() >= 4 && path.substr(path.size()-4) == ".css") return "text/css; charset=utf-8";
    if (path.size() >= 5 && path.substr(path.size()-5) == ".json") return "application/json; charset=utf-8";
    return "application/octet-stream";
}

static std::string http_response(int code, const std::string &status, const std::string &type, const std::string &body) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << code << " " << status << "\r\n";
    ss << "Content-Type: " << type << "\r\n";
    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << body;
    return ss.str();
}

static void handle_client(SOCKET client) {
    std::string req;
    char buf[4096];
    int received = 0;
    while ((received = recv(client, buf, sizeof(buf), 0)) > 0) {
        req.append(buf, buf + received);
        if (req.find("\r\n\r\n") != std::string::npos) break;
    }
    if (req.empty()) return;

    std::istringstream ss(req);
    std::string method, path, version;
    ss >> method >> path >> version;

    std::string headers = req.substr(0, req.find("\r\n\r\n"));
    size_t clen_pos = headers.find("Content-Length:");
    size_t content_length = 0;
    if (clen_pos != std::string::npos) {
        size_t end = headers.find("\r\n", clen_pos);
        std::string v = headers.substr(clen_pos + 15, end - (clen_pos + 15));
        content_length = static_cast<size_t>(std::strtoul(v.c_str(), nullptr, 10));
    }

    std::string body;
    size_t header_end = req.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        body = req.substr(header_end + 4);
        while (body.size() < content_length && (received = recv(client, buf, sizeof(buf), 0)) > 0) {
            body.append(buf, buf + received);
        }
    }

    if (method == "GET") {
        std::string file_path;
        if (path == "/" || path.empty()) {
            file_path = "index.html";
        } else {
            if (path.find("..") != std::string::npos) {
                std::string resp = http_response(400, "Bad Request", "text/plain", "Bad path");
                send(client, resp.c_str(), (int)resp.size(), 0);
                return;
            }
            if (!path.empty() && path[0] == '/') path = path.substr(1);
            file_path = path;
        }

        std::string data = read_file(file_path);
        if (data.empty()) {
            std::string resp = http_response(404, "Not Found", "text/plain", "Not Found");
            send(client, resp.c_str(), (int)resp.size(), 0);
            return;
        }
        std::string resp = http_response(200, "OK", content_type_for(file_path), data);
        send(client, resp.c_str(), (int)resp.size(), 0);
        return;
    }

    if (method == "POST" && path == "/bmi") {
        auto form = parse_form(body);
        if (form.find("weight") == form.end() || form.find("height") == form.end()) {
            std::string resp = http_response(400, "Bad Request", "application/json", "{\"error\":\"missing weight/height\"}");
            send(client, resp.c_str(), (int)resp.size(), 0);
            return;
        }

        double w = std::strtod(form["weight"].c_str(), nullptr);
        double h = std::strtod(form["height"].c_str(), nullptr);
        if (w <= 0.0 || h <= 0.0) {
            std::string resp = http_response(400, "Bad Request", "application/json", "{\"error\":\"weight and height must be positive\"}");
            send(client, resp.c_str(), (int)resp.size(), 0);
            return;
        }

        double bmi = w / (h * h);
        std::string category;
        if (bmi < 18.5) category = "Underweight";
        else if (bmi < 25.0) category = "Normal";
        else if (bmi < 30.0) category = "Overweight";
        else category = "Obese";

        char bmi_buf[32];
        std::snprintf(bmi_buf, sizeof(bmi_buf), "%.1f", bmi);

        std::ostringstream out;
        out << "{\"bmi\":" << bmi_buf << ",\"category\":\"" << category << "\"}";
        std::string resp = http_response(200, "OK", "application/json; charset=utf-8", out.str());
        send(client, resp.c_str(), (int)resp.size(), 0);
        return;
    }

    std::string resp = http_response(405, "Method Not Allowed", "text/plain", "Method Not Allowed");
    send(client, resp.c_str(), (int)resp.size(), 0);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::printf("WSAStartup failed\n");
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) {
        std::printf("socket failed\n");
        WSACleanup();
        return 1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::printf("bind failed\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    if (listen(server, 10) == SOCKET_ERROR) {
        std::printf("listen failed\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    std::printf("BMI server running on http://localhost:8080\n");

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;
        handle_client(client);
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
