#!/usr/bin/php
<?php
header('Status: 500 Internal Server Error');
?>
<!DOCTYPE html>
<html>
<head>
    <title>500 Internal Server Error</title>
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
        <h1 class="error-code">500</h1>
        <h2 class="error-title">Internal Server Error</h2>
        <p class="error-message">The server encountered an internal error and was unable to complete your request.</p>
        <p><a href="/">Return to Homepage</a></p>
    </div>
</body>
</html> 