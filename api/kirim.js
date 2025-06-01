export default async function handler(req, res) {
  if (req.method === 'POST') {
    try {
      const response = await fetch('https://script.google.com/macros/s/AKfycbzLfihyhb9Uzcov3H1hLJBaivZbLcEQ0thRnt0awMohH5JwjUZNQ6CuqaQS2jYbqLDFcQ/exec', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(req.body)
      });

      const result = await response.json();
      res.status(200).json(result);
    } catch (error) {
      res.status(500).json({ status: 'error', message: error.message });
    }
  } else {
    res.setHeader('Allow', ['POST']);
    res.status(405).end(`Method ${req.method} Not Allowed`);
  }
}
