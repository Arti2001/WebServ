# Use official Ubuntu base
FROM ubuntu:22.04

# Install build tools
RUN apt-get update && \
    apt-get install -y g++ make curl uuid-dev

# Create app directory
WORKDIR /app

# Copy everything into the container
COPY . .

# Compile the code when building the container
RUN make re

# Command to run your server by default
CMD ["./webserv" "/config-files/working/w.conf"]