# Use an official Python runtime as a parent image
FROM python:3.9-slim

# Install G++ for compiling the C++ automata engine
RUN apt-get update && apt-get install -y \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /app

# Copy the backend requirements file
COPY backend/requirements.txt .

# Install any needed packages specified in requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copy the entire project directory into the container
# We need the 'src' folder (C++ code) and the 'backend' folder
COPY . .

# Compile the C++ engine
# Note: Source is now in /app/src, output to /app/compiler_frontend
RUN g++ -std=c++14 -o /app/compiler_frontend src/*.cpp

# Expose the API port
EXPOSE 8000

# Run the FastAPI server
CMD ["uvicorn", "backend.main:app", "--host", "0.0.0.0", "--port", "8000"]
