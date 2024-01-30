const express = require('express');
const fs = require('fs').promises;
const path = require('path');

const app = express();
const logDirectory = '';

app.use(express.static('public'));

app.get('/:subdirectory', async (req, res) => {
  try {
    const subdirectory = req.params.subdirectory;
    const subdirectoryPath = path.join(logDirectory, subdirectory);

    // Read logs in the specified subdirectory
    const logFiles = await fs.readdir(subdirectoryPath);


    // Read and display the content of each log file in a monospace font
    const logsHtml = await Promise.all(logFiles.map(async (logFile) => {
      const logFilePath = path.join(subdirectoryPath, logFile);
      const logContent = await fs.readFile(logFilePath, 'utf-8');
      return `<div><h3>${logFile}</h3><pre>${logContent}</pre></div>`;
    }));

    // Display the information on an HTML page
    res.send(`
      <!DOCTYPE html>
      <html>
        <head>
          <title>Logs for Subdirectory ${subdirectory}</title>
          <link rel="stylesheet" type="text/css" href="/style.css">
        </head>
        <body>
          <h1>Logs for Subdirectory ${subdirectory}</h1>
          ${logsHtml.join('')}
        </body>
      </html>
    `);
  } catch (err) {
    console.error(err);
    res.status(500).send('Internal Server Error');
  }
});

const PORT = 5554;
app.listen(PORT, () => {
  console.log(`Server is running at http://localhost:${PORT}`);
});
