#!/bin/sh
echo "Content-Type: text/plain"
echo ""
echo "CGI Shell Script Test"
echo "Current directory: $(pwd)"
echo "Script name: $0"
echo "Environment variables:"
env | sort
