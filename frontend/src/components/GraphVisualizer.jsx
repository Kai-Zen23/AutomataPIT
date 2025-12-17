import { useEffect, useState, useRef } from 'react';
import { Graphviz } from 'graphviz-react';

const GraphVisualizer = ({ dotString, activeStateId }) => {
    const [processedDot, setProcessedDot] = useState(dotString);
    const containerRef = useRef(null);
    const prevActiveStateIdRef = useRef(null);

    useEffect(() => {
        if (!dotString) return;

        let newDot = dotString;

        // Highlight active state
        if (activeStateId !== null && activeStateId !== undefined) {
            // Extract just the ID number if it comes in as "q0" or "0"
            // Our ID is usually just the number string "0", "1" etc.
            // But the DOT file uses labels like "q0" or "0".
            // We need to match what's in the DOT file.

            // Try matching "qX" first (NFA/DFA), then just "X" (PDA)
            let stateLabel = `q${activeStateId}`;
            // If the dot string doesn't contain "qX", try just "X"
            if (!dotString.includes(`  ${stateLabel} [`)) {
                stateLabel = `${activeStateId}`;
            }

            const target = `  ${stateLabel} [`;
            const replacement = `  ${stateLabel} [style="filled", fillcolor="#0ea5e9", fontcolor="white", `;

            if (newDot.includes(target)) {
                newDot = newDot.replace(target, replacement);
            } else {
                const styleLine = `  ${stateLabel} [style="filled", fillcolor="#0ea5e9", fontcolor="white"];\n`;
                newDot = newDot.replace('}', styleLine + '}');
            }
        }

        setProcessedDot(newDot);
    }, [dotString, activeStateId]);

    // Animation Effect
    useEffect(() => {
        if (!containerRef.current || activeStateId === null || prevActiveStateIdRef.current === null) {
            prevActiveStateIdRef.current = activeStateId;
            return;
        }

        const fromId = prevActiveStateIdRef.current;
        const toId = activeStateId;

        // Animation temporarily disabled due to stability issues
        prevActiveStateIdRef.current = activeStateId;

    }, [activeStateId, processedDot]); // processedDot dependency ensures we re-bind if graph changes

    if (!dotString) return <div className="text-slate-500 italic p-4">No graph data available.</div>;

    return (
        <div ref={containerRef} className="graph-container overflow-auto bg-[#0B0F19] p-4 rounded-xl border border-slate-800 h-full flex items-center justify-center">
            <Graphviz dot={processedDot} options={{ height: "100%", width: "100%", fit: true, zoom: true }} />
        </div>
    );
};

export default GraphVisualizer;
