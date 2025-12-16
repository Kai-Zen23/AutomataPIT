# Automata Visualizer - Web Deployment Guide

This project has been converted into a full-stack web application.

## 1. Backend Deployment (Render)

This service runs the Python FastAPI server and compiles your C++ code.

1.  Push this repository to GitHub.
2.  Go to [dashboard.render.com](https://dashboard.render.com).
3.  Click **New +** -> **Web Service**.
4.  Connect your GitHub repository.
5.  **Settings**:
    *   **Runtime**: Docker
    *   **Build Command**: (Leave blank, it uses Dockerfile)
    *   **Start Command**: (Leave blank, it uses Dockerfile)
6.  Click **Deploy**.
7.  Once live, copy the **Service URL** (e.g., `https://automata-backend.onrender.com`).

## 2. Frontend Deployment (GitHub Pages)

The frontend is automated via GitHub Actions.

1.  Go to your GitHub Repository **Settings** -> **Secrets and variables** -> **Actions**.
2.  Click **New repository secret**.
3.  **Name**: `VITE_API_URL`
4.  **Value**: Your Render Service URL (from Step 1), e.g., `https://automata-backend.onrender.com`.  
    *(Note: Do not include a trailing slash)*.
5.  Go to the **Actions** tab in your repo.
6.  Select the "Deploy Frontend" workflow and click **Run workflow** (or just push a commit to `main`).
7.  Go to **Settings** -> **Pages**.
8.  Ensure "Build and deployment" source is set to **GitHub Actions**.

## 3. Local Development

To run the project on your machine:

**Backend**:
```bash
# Need Docker or manually install Python + G++
cd backend
pip install -r requirements.txt
uvicorn main:app --reload
```

**Frontend**:
```bash
cd frontend
npm install
npm run dev
```
