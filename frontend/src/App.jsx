import { useState, useEffect, useRef } from 'react';

const API_URL = import.meta.env.VITE_API_URL || 'http://localhost:8000';

function App() {
  const [mode, setMode] = useState(3); // 1: Regex, 3: Calculator
  const [input, setInput] = useState('x = 5 + 3');
  const [output, setOutput] = useState('');
  const [trace, setTrace] = useState([]);
  const [step, setStep] = useState(-1);
  const [isPlaying, setIsPlaying] = useState(false);
  const [speed, setSpeed] = useState(500);
  const [loading, setLoading] = useState(false);
  const playRef = useRef(null);

  // Auto-play loop
  useEffect(() => {
    if (isPlaying) {
      playRef.current = setInterval(() => {
        setStep((prev) => {
          if (prev < trace.length - 1) return prev + 1;
          setIsPlaying(false);
          return prev;
        });
      }, speed);
    } else {
      clearInterval(playRef.current);
    }
    return () => clearInterval(playRef.current);
  }, [isPlaying, trace.length, speed]);

  const handleSimulate = async () => {
    setLoading(true);
    setTrace([]);
    setStep(-1);
    setIsPlaying(false);
    setOutput('');

    try {
      const res = await fetch(`${API_URL}/simulate`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mode, input_text: input }),
      });
      const data = await res.json();

      if (data.error) {
        setOutput(`Error: ${data.error}\n${data.stderr || ''}`);
      } else {
        setOutput(data.stdout);
        if (data.trace) {
          setTrace(data.trace);
          // Auto-switch to visualizer if traces exist
          if (data.trace.length > 0) setStep(-1);
        }
      }
    } catch (err) {
      setOutput(`Network Error: ${err.message}`);
    } finally {
      setLoading(false);
    }
  };

  const getVisibleStack = () => {
    if (step < 0 || !trace[step] || trace[step].type !== 'stack') return [];
    // In a real implementation we would accumulate the stack actions
    // But our parser currently returns "Stack Content" as a string in the trace event
    // So we can just show the current row.
    return trace.slice(0, step + 1).filter(t => t.type === 'stack');
  };

  const getVisibleTokens = () => {
    // Show tokens accumulated up to current step
    if (step < 0) return [];
    // This logic depends on when tokens appear in the trace vs stack actions
    // For now, let's just show all tokens that have appeared so far
    return trace.slice(0, step + 1).filter(t => t.type === 'token');
  }

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 p-8 font-sans selection:bg-sky-500/30">
      <div className="max-w-6xl mx-auto space-y-8">

        {/* Header */}
        <header className="flex items-center justify-between border-b border-slate-800 pb-6">
          <div>
            <h1 className="text-3xl font-bold bg-gradient-to-r from-sky-400 to-indigo-400 bg-clip-text text-transparent">
              Automata Visualizer
            </h1>
            <p className="text-slate-400 mt-2">Interactive C++ Compiler Frontend Simulator</p>
          </div>
          <div className="flex space-x-4 bg-slate-900 p-1 rounded-lg border border-slate-800">
            <button
              onClick={() => setMode(1)}
              className={`px-4 py-2 rounded-md transition-all ${mode === 1 ? 'bg-sky-600 text-white shadow-lg shadow-sky-900/20' : 'text-slate-400 hover:text-white'}`}
            >
              Regex Analysis
            </button>
            <button
              onClick={() => setMode(3)}
              className={`px-4 py-2 rounded-md transition-all ${mode === 3 ? 'bg-sky-600 text-white shadow-lg shadow-sky-900/20' : 'text-slate-400 hover:text-white'}`}
            >
              Calculator (PDA)
            </button>
          </div>
        </header>

        {/* Input Area */}
        <section className="bg-slate-900/50 rounded-xl border border-slate-800 p-6 backdrop-blur-sm">
          <label className="block text-sm font-medium text-slate-400 mb-2">Input Expression</label>
          <div className="flex gap-4">
            <input
              type="text"
              value={input}
              onChange={(e) => setInput(e.target.value)}
              className="flex-1 bg-slate-950 border border-slate-700 rounded-lg px-4 py-3 font-mono text-lg focus:outline-none focus:ring-2 focus:ring-sky-500/50 transition-all placeholder-slate-600"
              placeholder={mode === 1 ? "[a-z]+" : "x = 5 + 3"}
              onKeyDown={(e) => e.key === 'Enter' && handleSimulate()}
            />
            <button
              onClick={handleSimulate}
              disabled={loading}
              className="bg-sky-600 hover:bg-sky-500 text-white px-8 py-3 rounded-lg font-semibold transition-all disabled:opacity-50 disabled:cursor-not-allowed shadow-lg shadow-sky-900/20 flex items-center gap-2"
            >
              {loading ? 'Compiling...' : 'Generate Trace'}
            </button>
          </div>
        </section>

        {/* Main Workspace */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 h-[600px]">

          {/* Left: Visualization */}
          <section className="bg-slate-900 rounded-xl border border-slate-800 flex flex-col overflow-hidden">
            <div className="p-4 border-b border-slate-800 flex justify-between items-center bg-slate-900/80">
              <h2 className="font-semibold text-slate-200">Simulation View</h2>
              <div className="flex items-center gap-2">
                <div className="px-3 py-1 bg-slate-800 rounded text-xs font-mono text-sky-400">
                  Step: {step + 1} / {trace.length}
                </div>
              </div>
            </div>

            <div className="flex-1 overflow-auto p-4 space-y-4 bg-[#0B0F19]">
              {/* Stack Table */}
              {trace.length > 0 ? (
                <div className="space-y-2">
                  {getVisibleStack().map((t, i) => (
                    <div key={i} className="flex items-center gap-3 p-3 rounded border border-slate-800 bg-slate-900/50 hover:bg-slate-800/50 transition-colors group animate-in fade-in slide-in-from-left-4 duration-300">
                      <div className="w-8 h-8 flex items-center justify-center rounded-full bg-slate-800 text-xs font-mono text-slate-400 group-hover:bg-sky-900/30 group-hover:text-sky-400">
                        {t.data.step}
                      </div>
                      <div className="flex-1 font-mono text-sm">
                        <div className="flex justify-between text-slate-400 text-xs mb-1">
                          <span>Idx: {t.data.input_index}</span>
                          <span className="text-emerald-400">{t.data.action}</span>
                        </div>
                        <div className="text-slate-200 bg-slate-950 px-2 py-1 rounded border border-slate-800/50">
                          {t.data.stack_content}
                        </div>
                      </div>
                    </div>
                  ))}
                  <div ref={(el) => el?.scrollIntoView({ behavior: 'smooth' })} />
                </div>
              ) : (
                <div className="h-full flex flex-col items-center justify-center text-slate-600 space-y-4">
                  <div className="w-16 h-16 rounded-full bg-slate-900 border border-slate-800 flex items-center justify-center text-2xl">
                    ⚡
                  </div>
                  <p>Run a simulation to see the stack trace here.</p>
                </div>
              )}
            </div>

            {/* Controls */}
            <div className="p-4 border-t border-slate-800 bg-slate-900/80 backdrop-blur">
              <div className="flex items-center justify-between gap-4">
                <div className="flex gap-2">
                  <button
                    onClick={() => setStep(s => Math.max(-1, s - 1))}
                    disabled={step < 0}
                    className="p-2 rounded hover:bg-slate-800 text-slate-400 hover:text-white disabled:opacity-30 disabled:hover:bg-transparent"
                  >
                    Prev
                  </button>
                  <button
                    onClick={() => setIsPlaying(!isPlaying)}
                    disabled={trace.length === 0}
                    className={`px-6 py-2 rounded font-medium transition-all ${isPlaying ? 'bg-amber-600/20 text-amber-400 border border-amber-600/50' : 'bg-emerald-600 text-white hover:bg-emerald-500 shadow-lg shadow-emerald-900/20'}`}
                  >
                    {isPlaying ? 'Pause' : 'Play'}
                  </button>
                  <button
                    onClick={() => setStep(s => Math.min(trace.length - 1, s + 1))}
                    disabled={step >= trace.length - 1}
                    className="p-2 rounded hover:bg-slate-800 text-slate-400 hover:text-white disabled:opacity-30 disabled:hover:bg-transparent"
                  >
                    Next
                  </button>
                </div>

                <div className="flex items-center gap-3 flex-1 justify-end">
                  <span className="text-xs text-slate-500 uppercase tracking-wider font-semibold">Speed</span>
                  <input
                    type="range"
                    min="100"
                    max="2000"
                    step="100"
                    value={speed}
                    onChange={(e) => setSpeed(Number(e.target.value))} // Reversed logic visual fix needed usually, low is fast
                    className="w-32 accent-sky-500"
                  />
                </div>
              </div>
            </div>
          </section>

          {/* Right: Console Output */}
          <section className="bg-slate-900 rounded-xl border border-slate-800 flex flex-col overflow-hidden">
            <div className="p-4 border-b border-slate-800 bg-slate-900/80">
              <h2 className="font-semibold text-slate-200">Execution Log</h2>
            </div>
            <div className="flex-1 overflow-auto p-0 bg-slate-950">
              <pre className="p-4 font-mono text-sm text-slate-300 whitespace-pre-wrap leading-relaxed">
                {output || <span className="text-slate-600 italic">// Console output will appear here...</span>}
              </pre>
            </div>
          </section>

        </div>
      </div>
    </div>
  );
}

export default App;
