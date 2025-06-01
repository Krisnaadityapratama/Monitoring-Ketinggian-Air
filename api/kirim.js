// File: api/kirim.js

export default async function handler(req, res) {
  if (req.method !== "POST") {
    return res.status(405).json({ error: "Metode hanya POST" });
  }

  try {
    const { sensor1, sensor2 } = req.body;

    const response = await fetch("https://script.google.com/macros/s/AKfycbzLfihyhb9Uzcov3H1hLJBaivZbLcEQ0thRnt0awMohH5JwjUZNQ6CuqaQS2jYbqLDFcQ/exec", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({ sensor1, sensor2 })
    });

    const result = await response.json();
    return res.status(200).json(result);
  } catch (error) {
    console.error("GAGAL KIRIM KE GOOGLE SCRIPT:", error);
    return res.status(500).json({ error: "Gagal kirim ke Google Script", detail: error.message });
  }
}
