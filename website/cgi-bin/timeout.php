#!/usr/bin/php
<?php

// Get timeout duration from query string (default 30 seconds)
$timeout = isset($_GET['timeout']) ? intval($_GET['timeout']) : 5;

// Cap maximum timeout at 60 seconds
$timeout = min($timeout, 10);

// Sleep for the specified duration
sleep($timeout);

// Send timeout response
header('Status: 504 Gateway Timeout');
?>
<!DOCTYPE html>
<html>
<head>
    <title>504 Gateway Timeout</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            line-height: 1.6;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            text-align: center;
        }
        .error-container {
            border: 1px solid #ddd;
            padding: 20px;
            border-radius: 5px;
            background-color: #f8f8f8;
        }
        .error-code {
            font-size: 72px;
            color: #e74c3c;
            margin: 0;
        }
        .error-title {
            font-size: 24px;
            color: #333;
            margin-top: 0;
        }
        .error-message {
            color: #555;
        }
    </style>
</head>
<body>
    <div class="error-container">
        <h1 class="error-code">504</h1>
        <h2 class="error-title">Gateway Timeout</h2>
        <p class="error-message">The server timed out while processing your request.</p>
        <p>Timeout duration: <?php echo $timeout; ?> seconds</p>
        <p><a href="/">Return to Homepage</a></p>
    </div>
</body>
</html> 