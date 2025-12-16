from backend.parser import parse_trace
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import subprocess
import os

app = FastAPI()

# Allow CORS for React Frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class SimulationRequest(BaseModel):
    mode: int
    input_text: str
    test_string: str = "myVar" # Default for backward compatibility

@app.get("/")
def read_root():
    return {"status": "Automata Visualizer API is running"}

@app.post("/simulate")
def run_simulation(req: SimulationRequest):
    # Determine executable path
    # In Docker, it's at /app/compiler_frontend
    # Locally, it might be in root
    exe_path = "/app/compiler_frontend"
    if not os.path.exists(exe_path):
        exe_path = "./compiler_frontend.exe" # Fallback for local Windows testing if compiled
    
    if not os.path.exists(exe_path):
        # Development fallback: if exe not found, mock it or return error?
        # For now return error but maybe detailed
        return {"error": "Compiler executable not found. Please compile src/ first."}
        
    # Input format:
    # Line 1: Mode
    # Line 2: Regex / Input
    # Line 3: Test String
    input_str = f"{req.mode}\n{req.input_text}\n{req.test_string}\n"
    
    try:
        result = subprocess.run(
            [exe_path], 
            input=input_str, 
            text=True, 
            capture_output=True, 
            check=True
        )
        
        parsed_data = []
        if req.mode == 1 or req.mode == 3:
             parsed_data = parse_trace(result.stdout)

        # Read generated graph files
        graphs = {}
        for graph_type in ["nfa", "dfa", "min_dfa", "pda"]:
            dot_path = f"{graph_type}.dot"
            # In Docker, files are in /app/, locally they are in root.
            # subprocess.run assumes cwd is root, so files should be there.
            if os.path.exists(dot_path):
                with open(dot_path, "r") as f:
                    graphs[graph_type] = f.read()

        return {
            "stdout": result.stdout,
            "stderr": result.stderr,
            "exit_code": result.returncode,
            "trace": parsed_data,
            "graphs": graphs
        }
    except subprocess.CalledProcessError as e:
        return {
            "error": "Execution failed",
            "stdout": e.stdout,
            "stderr": e.stderr
        }
    except Exception as e:
        return {"error": str(e)}
