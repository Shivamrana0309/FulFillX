import React, { useState } from 'react';

const DispatchPanel = () => {
  const [formData, setFormData] = useState({
    driver_id: '',
    warehouse_node: '',
    max_deadline: ''
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [routeResult, setRouteResult] = useState(null);

  const handleInputChange = (e) => {
    const { name, value } = e.target;
    setFormData(prev => ({
      ...prev,
      [name]: value
    }));
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setError(null);
    setRouteResult(null);

    // Basic validation
    if (!formData.driver_id || !formData.warehouse_node || !formData.max_deadline) {
      setError('All fields are required.');
      setLoading(false);
      return;
    }

    try {
      const response = await fetch('http://localhost:8080/api/dispatch', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          driver_id: parseInt(formData.driver_id, 10),
          warehouse_node: parseInt(formData.warehouse_node, 10),
          max_deadline: parseInt(formData.max_deadline, 10)
        })
      });

      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.message || data.error || `HTTP error! status: ${response.status}`);
      }

      setRouteResult(data.route);
    } catch (err) {
      setError(err.message || 'Failed to dispatch. Ensure the FulfillX Engine is running.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="flex flex-col h-full bg-[#1c2128]">
      {/* Header */}
      <div className="border-b border-gray-800 bg-[#22272e] px-5 py-3 flex justify-between items-center">
        <h2 className="text-sm font-semibold text-white uppercase tracking-wider">Dispatch Control</h2>
      </div>

      <div className="p-6 flex-1 flex flex-col space-y-6 overflow-auto">
        
        {/* Error Banner */}
        {error && (
          <div className="flex items-center space-x-3 rounded-lg border border-red-900/50 bg-red-500/10 p-4 text-red-400 shadow-sm shadow-red-900/20">
            <svg className="h-5 w-5 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
            </svg>
            <p className="text-sm font-medium">{error}</p>
          </div>
        )}

        {/* Dispatch Form */}
        <form onSubmit={handleSubmit} className="rounded-xl border border-gray-800 bg-[#22272e] p-5 shadow-sm">
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-5">
            <div>
              <label className="mb-1 block text-xs font-medium text-gray-400">Driver ID</label>
              <input
                type="number"
                name="driver_id"
                value={formData.driver_id}
                onChange={handleInputChange}
                placeholder="e.g. 101"
                className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
              />
            </div>
            <div>
              <label className="mb-1 block text-xs font-medium text-gray-400">Warehouse Node</label>
              <input
                type="number"
                name="warehouse_node"
                value={formData.warehouse_node}
                onChange={handleInputChange}
                placeholder="e.g. 1"
                className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
              />
            </div>
            <div>
              <label className="mb-1 block text-xs font-medium text-gray-400">Max Deadline (Unix)</label>
              <input
                type="number"
                name="max_deadline"
                value={formData.max_deadline}
                onChange={handleInputChange}
                placeholder="e.g. 1700005000"
                className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
              />
            </div>
          </div>
          <button
            type="submit"
            disabled={loading}
            className="w-full flex items-center justify-center space-x-2 rounded bg-purple-600 px-4 py-2.5 text-sm font-bold text-white shadow-md shadow-purple-500/20 hover:bg-purple-500 transition-colors disabled:opacity-50"
          >
            {loading && (
              <svg className="h-4 w-4 animate-spin" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
              </svg>
            )}
            <span>Run Dijkstra Engine</span>
          </button>
        </form>

        {/* Route Visualization */}
        {routeResult ? (
          <div className="flex-1 rounded-xl border border-green-900/30 bg-gradient-to-b from-[#161b22] to-[#1c2128] p-6 shadow-lg relative overflow-hidden">
            {/* Background Accent */}
            <div className="absolute top-0 left-0 w-full h-1 bg-gradient-to-r from-green-500 to-emerald-400"></div>
            
            <h3 className="text-lg font-bold text-white mb-6 flex items-center space-x-2">
              <svg className="h-5 w-5 text-green-400" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
              </svg>
              <span>Route Generated Successfully</span>
            </h3>

            {/* Summary Metrics */}
            <div className="grid grid-cols-2 gap-4 mb-8">
              <div className="rounded-lg border border-gray-800 bg-[#22272e] p-4 text-center">
                <p className="text-xs font-medium text-gray-400 uppercase tracking-wide">Total Distance</p>
                <p className="mt-1 text-3xl font-extrabold text-green-400">
                  {routeResult.total_distance_km.toFixed(1)} <span className="text-sm font-medium text-gray-500">km</span>
                </p>
              </div>
              <div className="rounded-lg border border-gray-800 bg-[#22272e] p-4 text-center">
                <p className="text-xs font-medium text-gray-400 uppercase tracking-wide">Packages Delivered</p>
                <p className="mt-1 text-3xl font-extrabold text-white">
                  {routeResult.delivered_package_ids?.length || 0}
                </p>
              </div>
            </div>

            {/* Timeline Flowchart */}
            <div>
              <p className="text-xs font-medium text-gray-400 uppercase tracking-wide mb-4">Optimal Path Sequence</p>
              <div className="flex items-center space-x-2 overflow-x-auto pb-4 custom-scrollbar">
                {routeResult.path_nodes.map((nodeId, index) => (
                  <React.Fragment key={index}>
                    <div className="flex shrink-0 flex-col items-center justify-center h-12 w-12 rounded-full border-2 border-purple-500 bg-purple-500/10 text-purple-300 font-bold shadow-[0_0_15px_rgba(168,85,247,0.15)]">
                      {nodeId}
                    </div>
                    {index < routeResult.path_nodes.length - 1 && (
                      <div className="shrink-0 flex items-center text-gray-600 px-1">
                        <svg className="h-5 w-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M17 8l4 4m0 0l-4 4m4-4H3" />
                        </svg>
                      </div>
                    )}
                  </React.Fragment>
                ))}
              </div>
            </div>

          </div>
        ) : (
          !loading && !error && (
            <div className="flex-1 flex flex-col items-center justify-center text-center p-6 border-2 border-dashed border-gray-800 rounded-xl">
              <div className="rounded-full bg-gray-800 p-4 mb-4">
                <svg className="w-8 h-8 text-gray-500" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth="1.5" d="M9 20l-5.447-2.724A1 1 0 013 16.382V5.618a1 1 0 011.447-.894L9 7m0 13l6-3m-6 3V7m6 10l4.553 2.276A1 1 0 0021 18.382V7.618a1 1 0 00-.553-.894L15 4m0 13V4m0 0L9 7"></path></svg>
              </div>
              <p className="font-medium text-gray-300">Ready for Dispatch</p>
              <p className="text-xs text-gray-500 mt-1">Configure parameters and run the engine to visualize the shortest path.</p>
            </div>
          )
        )}
      </div>
    </div>
  );
};

export default DispatchPanel;
