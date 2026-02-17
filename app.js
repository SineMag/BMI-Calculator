const form = document.getElementById('bmi-form');
const result = document.getElementById('result');

function setResult(html, kind) {
  result.className = `result ${kind}`;
  result.innerHTML = html;
}

form.addEventListener('submit', async (e) => {
  e.preventDefault();
  const weight = document.getElementById('weight').value;
  const height = document.getElementById('height').value;

  setResult('Calculating...', 'pending');

  try {
    const res = await fetch('/bmi', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: `weight=${encodeURIComponent(weight)}&height=${encodeURIComponent(height)}`
    });

    const data = await res.json();
    if (!res.ok) {
      setResult(`<strong>Error:</strong> ${data.error || 'Request failed'}`, 'error');
      return;
    }

    setResult(
      `<div class="bmi">BMI: <strong>${data.bmi}</strong></div>
       <div class="category">Category: <strong>${data.category}</strong></div>`,
      'success'
    );
  } catch (err) {
    setResult('<strong>Error:</strong> Could not reach server.', 'error');
  }
});
