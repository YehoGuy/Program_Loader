# Use a base image compatible with 32-bit architecture
FROM i386/ubuntu:20.04

# Update and install necessary packages
RUN apt-get update || (sleep 30 && apt-get update) && apt-get install -y --fix-missing \
    build-essential \
    gcc-multilib \
    g++-multilib \
    vim


# Set a working directory
WORKDIR /app

# Copy the current directory's contents (including Makefile and source code) to the container
COPY . /app

# Set the default command to bash
CMD ["/bin/bash"]

# build the Docker image with: "docker build -t my-32bit-env ."
# Run the container and mount your working directory: "docker run -it --rm -v $(pwd):/app my-32bit-env"
# Navigate to the /app directory if not already there. and "make".