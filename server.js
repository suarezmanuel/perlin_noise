const express = require('express');
const path = require('path');
const app = express();
const port = 5500;

// 1. Add COOP and COEP headers before serving any content
app.use((req, res, next) => {
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    next();
});

// 2. Serve static files from the current directory
app.use(express.static(__dirname));

// 3. Fallback route to serve index.html for any other requests
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

app.listen(port, () => {
    console.log(`Server running at http://127.0.0.1:${port}`);
});
