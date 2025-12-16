import { useEffect, useState } from 'react';
import { Graphviz } from 'graphviz-react';

const GraphVisualizer = ({ dotString, activeStateId }) => {
    const [processedDot, setProcessedDot] = useState(dotString);

    useEffect(() => {
        if (!dotString) return;

        let newDot = dotString;

        // Highlight active state
        // We look for the line defining the node, e.g. "q0 [shape=circle];"
        // and inject style="filled", fillcolor="yellow" (or a project theme color)

        // Node format in our C++: "  ID [shape=...];" or "  ID -> ID ..."
        // We want to target the node definition itself if possible. 
        // Usually "  q0 [shape=circle];" or "  q0 [shape=doublecircle];"
        // Or sometimes just implicit. But our DFA.cpp explicitly defines shapes.

        if (activeStateId !== null && activeStateId !== undefined) {
            // Create a regex to find the node definition
            // Pattern: start of line, whitespace, stateID, whitespace, [attributes]

            const stateLabel = `q${activeStateId}`;

            // Simple string replacement might be safer than complex regex for DOT
            // We want to replace `qX [` with `qX [style="filled", fillcolor="#0ea5e9", fontcolor="white", `
            // The color #0ea5e9 is our Sky-500 from Tailwind theme.

            const target = `  ${stateLabel} [`;
            const replacement = `  ${stateLabel} [style="filled", fillcolor="#0ea5e9", fontcolor="white", `;

            if (newDot.includes(target)) {
                newDot = newDot.replace(target, replacement);
            } else {
                // Fallback: if node is not explicitly defined with attributes (unlikely in our code), 
                // append a style line.
                // "}" is the end of the digraph. Insert before it.
                const styleLine = `  ${stateLabel} [style="filled", fillcolor="#0ea5e9", fontcolor="white"];\n`;
                newDot = newDot.replace('}', styleLine + '}');
            }
        }

        setProcessedDot(newDot);
    }, [dotString, activeStateId]);

    if (!dotString) return <div className="text-slate-500 italic p-4">No graph data available.</div>;

    return (
        <div className="graph-container overflow-auto bg-[#0B0F19] p-4 rounded-xl border border-slate-800 h-full flex items-center justify-center">
            <Graphviz dot={processedDot} options={{ height: "100%", width: "100%", fit: true, zoom: true }} />
        </div>
    );
};

export default GraphVisualizer;
