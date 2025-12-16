import tkinter as tk
from tkinter import ttk, messagebox, PhotoImage
import subprocess
import os
import sys
import re

class AutomataVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("Automata Compiler Visualizer")
        self.root.geometry("1100x850")

        # Config
        self.exe_path = "compiler_frontend.exe"
        if not os.path.exists(self.exe_path):
             if os.path.exists(os.path.join("src", "compiler_frontend.exe")):
                 self.exe_path = os.path.join("src", "compiler_frontend.exe")

        # Simulation State
        self.trace = []
        self.current_step = -1
        self.is_playing = False
        self.play_speed = 500 # ms

        # Styles
        style = ttk.Style()
        style.theme_use('clam')

        # Control Panel
        control_frame = ttk.LabelFrame(root, text="Input", padding="10")
        control_frame.pack(fill=tk.X, padx=10, pady=5)

        # Input Row
        input_frame = ttk.Frame(control_frame)
        input_frame.pack(fill=tk.X, pady=2)

        # Mode Selection
        mode_frame = ttk.Frame(control_frame)
        mode_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(mode_frame, text="Mode: ").pack(side=tk.LEFT)
        self.mode_var = tk.IntVar(value=3) # Default to Calculator
        ttk.Radiobutton(mode_frame, text="Regex Analysis (NFA/DFA)", variable=self.mode_var, value=1).pack(side=tk.LEFT, padx=5)
        ttk.Radiobutton(mode_frame, text="Calculator (PDA)", variable=self.mode_var, value=3).pack(side=tk.LEFT, padx=5)

        ttk.Label(input_frame, text="Enter Input:").pack(side=tk.LEFT)
        self.regex_entry = ttk.Entry(input_frame, width=40, font=('Consolas', 11))
        self.regex_entry.pack(side=tk.LEFT, padx=10)
        self.regex_entry.insert(0, "x = 5 + 3")
        self.regex_entry.bind('<Return>', lambda e: self.run_visualization())

        self.btn_run = ttk.Button(input_frame, text="Generate Trace", command=self.run_visualization)
        self.btn_run.pack(side=tk.LEFT, padx=10)

        # Symbol Toolbar
        toolbar_frame = ttk.Frame(control_frame)
        toolbar_frame.pack(fill=tk.X, pady=2)
        
        symbols = ['*', '|', '(', ')', '+', '?', '[a-z]', '[0-9]', '=', 'x', 'y', 'z']
        for sym in symbols:
            btn = ttk.Button(toolbar_frame, text=sym, width=5, command=lambda s=sym: self.insert_symbol(s))
            btn.pack(side=tk.LEFT, padx=2)

        # Simulation Controls
        sim_frame = ttk.LabelFrame(root, text="Simulation Controls", padding="10")
        sim_frame.pack(fill=tk.X, padx=10, pady=5)

        self.btn_prev = ttk.Button(sim_frame, text="<< Prev", command=self.prev_step, state=tk.DISABLED)
        self.btn_prev.pack(side=tk.LEFT, padx=5)

        self.btn_play = ttk.Button(sim_frame, text="Play", command=self.toggle_play, state=tk.DISABLED)
        self.btn_play.pack(side=tk.LEFT, padx=5)

        self.btn_next = ttk.Button(sim_frame, text="Next >>", command=self.next_step, state=tk.DISABLED)
        self.btn_next.pack(side=tk.LEFT, padx=5)
        
        self.btn_reset = ttk.Button(sim_frame, text="Reset", command=self.reset_simulation, state=tk.DISABLED)
        self.btn_reset.pack(side=tk.LEFT, padx=5)

        ttk.Label(sim_frame, text="Speed:").pack(side=tk.LEFT, padx=10)
        self.speed_scale = ttk.Scale(sim_frame, from_=2000, to=100, orient=tk.HORIZONTAL, length=200, command=self.update_speed)
        self.speed_scale.set(500)
        self.speed_scale.pack(side=tk.LEFT, padx=5)
        
        self.lbl_step = ttk.Label(sim_frame, text="Step: 0 / 0")
        self.lbl_step.pack(side=tk.LEFT, padx=20)

        # Status Bar
        self.status_var = tk.StringVar()
        self.status_var.set("Ready")
        self.status_bar = ttk.Label(root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)

        # Tabs for Images
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.tab_nfa = self.create_tab("NFA")
        self.tab_dfa = self.create_tab("DFA")
        self.tab_min = self.create_tab("Minimized DFA")
        
        # New Tabs for Calculator
        self.tab_tokens = self.create_tree_tab("Tokens", ["Index", "Type", "Value"])
        self.tab_stack = self.create_tree_tab("Parser Stack", ["Step", "Input Index", "Action", "Stack Content"])
        self.tab_console = self.create_text_tab("Console Output")

        self.images = {}

    def update_speed(self, val):
        self.play_speed = int(float(val))

    def create_text_tab(self, title):
        frame = ttk.Frame(self.notebook)
        self.notebook.add(frame, text=title)
        
        text_area = tk.Text(frame, wrap="word", font=('Consolas', 10))
        scroll_y = ttk.Scrollbar(frame, orient="vertical", command=text_area.yview)
        text_area.configure(yscrollcommand=scroll_y.set)
        
        scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        text_area.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        frame.text_area = text_area
        return frame

    def create_tree_tab(self, title, columns):
        frame = ttk.Frame(self.notebook)
        self.notebook.add(frame, text=title)

        tree = ttk.Treeview(frame, columns=columns, show='headings')
        for col in columns:
            tree.heading(col, text=col)
            # Adjust column widths based on content
            width = 100
            if col in ["Stack Content", "Action"]:
               width = 300
            tree.column(col, width=width)

        scroll_y = ttk.Scrollbar(frame, orient="vertical", command=tree.yview)
        tree.configure(yscrollcommand=scroll_y.set)
        
        scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        frame.tree = tree
        return frame

    def create_tab(self, title):
        frame = ttk.Frame(self.notebook)
        self.notebook.add(frame, text=title)
        
        canvas = tk.Canvas(frame, bg='#f0f0f0')
        scroll_x = ttk.Scrollbar(frame, orient="horizontal", command=canvas.xview)
        scroll_y = ttk.Scrollbar(frame, orient="vertical", command=canvas.yview)
        canvas.configure(xscrollcommand=scroll_x.set, yscrollcommand=scroll_y.set)

        scroll_x.pack(side=tk.BOTTOM, fill=tk.X)
        scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        frame.canvas = canvas
        return frame

    def update_image(self, tab, filename):
        if not os.path.exists(filename):
            return
        try:
            img = PhotoImage(file=filename)
            self.images[filename] = img 
            canvas = tab.canvas
            canvas.delete("all")
            canvas.create_image(0, 0, image=img, anchor="nw")
            canvas.config(scrollregion=canvas.bbox("all"))
        except Exception as e:
            messagebox.showerror("Image Load Error", f"Failed to load {filename}: {e}")

    def insert_symbol(self, symbol):
        self.regex_entry.insert(tk.INSERT, symbol)
        self.regex_entry.focus()

    def run_visualization(self):
        regex = self.regex_entry.get()
        if not regex: return

        self.status_var.set("Running Compiler...")
        self.reset_ui_data()
        self.root.update()

        try:
            mode = self.mode_var.get()
            input_str = f"{mode}\n{regex}\n"
            startupinfo = None
            if os.name == 'nt':
                startupinfo = subprocess.STARTUPINFO()
                startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            
            # Look for the executable in current dir or src/
            exe_to_run = self.exe_path
            
            proc = subprocess.run([exe_to_run], input=input_str, text=True, capture_output=True, check=True, startupinfo=startupinfo)
            
            self.tab_console.text_area.delete(1.0, tk.END)
            self.tab_console.text_area.insert(tk.END, proc.stdout)
            
            # Enable animation for both Regex (1) and Calculator (3)
            if mode == 1 or mode == 3:
                self.parse_and_prepare_trace(proc.stdout)
            
            # For Regex mode, also regenerate graphs
            if mode == 1:
                 self.status_var.set("Generating Graphs and Trace...")
                 self.generate_graphs()

        except Exception as e:
            self.status_var.set("Error running compiler")
            messagebox.showerror("Execution Error", f"Failed to run: {e}")

    def parse_and_prepare_trace(self, output):
        self.trace = []
        token_counter = 0
        
        # Temporary buffers for multi-line step parsing
        current_step = None
        current_idx = None
        current_action = ""
        
        lines = output.splitlines()
        for i, line in enumerate(lines):
            line = line.strip()
            
            # 1. Parse Tokens: "Token: TYPE (VALUE)"
            if line.startswith("Token:"): 
                m = re.search(r"Token: (\w+) \((.*)\)", line)
                if m:
                    self.trace.append({
                        'type': 'token',
                        'data': (token_counter + 1, m.group(1), m.group(2))
                    })
                    token_counter += 1
            
            # 2. Parse Stack Step Header: "Step 1: Read 'x' at position 0"
            elif line.startswith("Step"): 
                m = re.search(r"Step (\d+): Read .* at position (\d+)", line)
                if m:
                    current_step = m.group(1)
                    current_idx = m.group(2)
                    current_action = "" # Reset action

            # 3. Parse Action: "Action: PUSH ..."
            elif line.startswith("Action:"):
                 current_action = line.replace("Action:", "").strip()

            # 4. Parse Stack: "Stack: [...]" -> Commit the row
            elif line.startswith("Stack: ["):
                if current_step and current_idx:
                    stack_content = line.replace("Stack: [", "").replace("]", "").strip()
                    self.trace.append({
                        'type': 'stack',
                        'data': (current_step, current_idx, current_action, stack_content)
                    })
                    # Prepare for next
                    current_step = None 
        
        if not self.trace:
             self.status_var.set("Finished (No PDA Trace Data)")
             return

        self.status_var.set(f"Loaded {len(self.trace)} steps. Ready to play.")
        self.notebook.select(self.tab_stack)
        
        self.btn_play.config(state=tk.NORMAL)
        self.btn_prev.config(state=tk.NORMAL)
        self.btn_next.config(state=tk.NORMAL)
        self.btn_reset.config(state=tk.NORMAL)
        self.current_step = -1
        self.update_step_label()

    def generate_graphs(self):
        files = [("nfa.dot", "nfa.png"), ("dfa.dot", "dfa.png"), ("min_dfa.dot", "min_dfa.png")]
        startupinfo = None
        if os.name == 'nt':
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            
        try:
            for dot, png in files:
                if os.path.exists(dot):
                    subprocess.run(["dot", "-Tpng", dot, "-o", png], check=True, startupinfo=startupinfo)
            self.update_image(self.tab_nfa, "nfa.png")
            self.update_image(self.tab_dfa, "dfa.png")
            self.update_image(self.tab_min, "min_dfa.png")
            self.status_var.set("Graph generation complete.")
        except Exception as e:
             self.status_var.set("Graphviz error")
             messagebox.showerror("Graphviz Error", f"{e}")

    def reset_ui_data(self):
        self.trace = []
        self.current_step = -1
        self.is_playing = False
        self.btn_play.config(text="Play", state=tk.DISABLED)
        self.btn_prev.config(state=tk.DISABLED)
        self.btn_next.config(state=tk.DISABLED)
        self.btn_reset.config(state=tk.DISABLED)
        
        for tab in [self.tab_tokens, self.tab_stack]:
            for item in tab.tree.get_children():
                tab.tree.delete(item)
        self.update_step_label()

    def toggle_play(self):
        if self.is_playing:
            self.is_playing = False
            self.btn_play.config(text="Play")
        else:
            if self.current_step >= len(self.trace) - 1:
                self.reset_simulation()
            self.is_playing = True
            self.btn_play.config(text="Pause")
            self.play_loop()

    def play_loop(self):
        if not self.is_playing: return
        
        if self.current_step < len(self.trace) - 1:
            self.next_step()
            self.root.after(self.play_speed, self.play_loop)
        else:
            self.is_playing = False
            self.btn_play.config(text="Play")
            self.status_var.set("Simulation Finished.")

    def next_step(self):
        if self.current_step < len(self.trace) - 1:
            self.current_step += 1
            self.process_step(self.current_step, direction=1)
            self.update_step_label()
            self.auto_scroll()

    def prev_step(self):
        if self.current_step >= 0:
            # Undo visual
            self.process_step(self.current_step, direction=-1)
            self.current_step -= 1
            self.update_step_label()

    def reset_simulation(self):
        self.is_playing = False
        self.btn_play.config(text="Play")
        
        # Clear UI but keep trace
        for tab in [self.tab_tokens, self.tab_stack]:
             for item in tab.tree.get_children():
                 tab.tree.delete(item)
        
        self.current_step = -1
        self.update_step_label()

    def process_step(self, index, direction):
        event = self.trace[index]
        if direction == 1:
            # Add to UI
            if event['type'] == 'token':
                self.tab_tokens.tree.insert('', tk.END, iid=f"t_{index}", values=event['data'])
                self.tab_tokens.tree.see(f"t_{index}")
                self.tab_tokens.tree.selection_set(f"t_{index}")
            elif event['type'] == 'stack':
                self.tab_stack.tree.insert('', tk.END, iid=f"s_{index}", values=event['data'])
                self.tab_stack.tree.see(f"s_{index}")
                self.tab_stack.tree.selection_set(f"s_{index}")
        else:
            # Remove from UI (Undo)
            if event['type'] == 'token':
                if self.tab_tokens.tree.exists(f"t_{index}"):
                    self.tab_tokens.tree.delete(f"t_{index}")
            elif event['type'] == 'stack':
                if self.tab_stack.tree.exists(f"s_{index}"):
                     self.tab_stack.tree.delete(f"s_{index}")

    def update_step_label(self):
        self.lbl_step.config(text=f"Step: {self.current_step + 1} / {len(self.trace)}")

    def auto_scroll(self):
        pass # Handle by 'see' in process_step

if __name__ == "__main__":
    if not os.path.exists("compiler_frontend.exe") and not os.path.exists("src/compiler_frontend.exe"):
        # We allow running just to see the GUI even if exe missing, but warn
        # messagebox.showwarning("Missing Executable", "Could not find compiler_frontend.exe.")
        pass
    
    root = tk.Tk()
    app = AutomataVisualizer(root)
    root.mainloop()
