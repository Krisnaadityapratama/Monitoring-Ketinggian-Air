export default async function handler(req, res) {
  if (req.method !== "POST") {
    return res.status(405).json({ error: "Metodee hanya POST" });
  }

  try {
    console.log("Body diterima:", req.body);

    const { sensor1, sensor2 } = req.body;

    if (sensor1 === undefined || sensor2 === undefined) {
      console.error("Data sensor tidak lengkap");
      return res.status(400).json({ error: "Data sensor1 atau sensor2 tidak ada" });
    }

    const response = await fetch("https://script.google.com/macros/s/AKfycbzLfihyhb9Uzcov3H1hLJBaivZbLcEQ0thRnt0awMohH5JwjUZNQ6CuqaQS2jYbqLDFcQ/exec", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({ sensor1, sensor2 })
    });

    if (!response.ok) {
      const text = await response.text();
      console.error("Response dari Google Script gagal:", response.status, text);
      return res.status(500).json({ error: "Google Script API error", status: response.status, text });
    }

    const result = await response.json();
    console.log("Response dari Google Script:", result);
    return res.status(200).json(result);

  } catch (error) {
    console.error("Error di handler:", error);
    return res.status(500).json({ error: "Gagal kirim ke Google Script", detail: error.message });
  }
}
