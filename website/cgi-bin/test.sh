#!/bin/sh
echo "Content-Type: text/html"
echo ""
echo "<pre>"
echo "CGI Shell Script Test"
echo "Current directory: $(pwd)"
echo "Script name: $0"
echo ""
echo "Environment variables:"
env | sort
echo "</pre>"
