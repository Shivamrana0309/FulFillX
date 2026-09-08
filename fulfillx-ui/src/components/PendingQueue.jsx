import React, { useState, useEffect } from 'react';

const PendingQueue = ({ refreshKey }) => {
  const [packages, setPackages] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchPendingPackages = async () => {
    setLoading(true);
    setError(null);
    try {
      const response = await fetch('http://localhost:8080/api/pending');
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.json();
      
      // Defensively parse the packages array
      // Note: Backend currently returns pending_count, so we fallback to empty array if packages is undefined
      setPackages(data.packages || []);
    } catch (err) {
      setError(err.message || 'Failed to connect to FulfillX Engine');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchPendingPackages();
  }, [refreshKey]);

  // Format UNIX timestamp (seconds) to readable string
  const formatTime = (unixSeconds) => {
    if (!unixSeconds) return 'N/A';
    const date = new Date(unixSeconds * 1000);
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };

  return (
    <div className="flex h-full flex-col bg-[#1c2128]">
      <div className="flex items-center justify-between border-b border-gray-800 bg-[#22272e] px-5 py-3">
        <h2 className="text-sm font-semibold uppercase tracking-wider text-white">
          Pending Queue
        </h2>
        <button
          onClick={fetchPendingPackages}
          disabled={loading}
          className="flex items-center space-x-1 rounded bg-gray-700 px-3 py-1.5 text-xs font-semibold text-white transition hover:bg-gray-600 disabled:opacity-50"
        >
          <svg
            className={`h-3.5 w-3.5 ${loading ? 'animate-spin' : ''}`}
            fill="none"
            stroke="currentColor"
            viewBox="0 0 24 24"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth="2"
              d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"
            />
          </svg>
          <span>Refresh</span>
        </button>
      </div>

      <div className="flex-1 overflow-auto">
        {loading && packages.length === 0 ? (
          <div className="flex h-full flex-col items-center justify-center space-y-3">
            <div className="h-6 w-6 animate-spin rounded-full border-2 border-purple-500 border-t-transparent"></div>
            <p className="text-sm text-gray-400">Syncing with Engine...</p>
          </div>
        ) : error ? (
          <div className="flex h-full flex-col items-center justify-center space-y-2 text-center p-6">
            <div className="rounded-full bg-red-900/30 p-3 text-red-500">
              <svg className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
              </svg>
            </div>
            <p className="text-sm font-medium text-red-400">Connection Error</p>
            <p className="text-xs text-red-500/70">{error}</p>
          </div>
        ) : packages.length === 0 ? (
          <div className="flex h-full flex-col items-center justify-center space-y-2 p-6 text-center">
            <div className="rounded-full bg-gray-800 p-4">
              <svg
                className="h-8 w-8 text-gray-500"
                fill="none"
                stroke="currentColor"
                viewBox="0 0 24 24"
              >
                <path
                  strokeLinecap="round"
                  strokeLinejoin="round"
                  strokeWidth="1.5"
                  d="M20 13V6a2 2 0 00-2-2H6a2 2 0 00-2 2v7m16 0v5a2 2 0 01-2 2H6a2 2 0 01-2-2v-5m16 0h-2.586a1 1 0 00-.707.293l-2.414 2.414a1 1 0 01-.707.293h-3.172a1 1 0 01-.707-.293l-2.414-2.414A1 1 0 006.586 13H4"
                ></path>
              </svg>
            </div>
            <p className="font-medium text-gray-300">No packages pending</p>
            <p className="text-xs text-gray-500">The delivery queue is currently empty.</p>
          </div>
        ) : (
          <table className="w-full text-left text-sm text-gray-400">
            <thead className="sticky top-0 bg-[#22272e] text-xs uppercase text-gray-500 shadow-sm">
              <tr>
                <th className="px-4 py-3 font-medium">ID</th>
                <th className="px-4 py-3 font-medium">Destination Node</th>
                <th className="px-4 py-3 font-medium">Priority</th>
                <th className="px-4 py-3 font-medium text-right">Deadline</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-800">
              {packages.map((pkg) => (
                <tr key={pkg.id} className="transition-colors hover:bg-gray-800/50">
                  <td className="px-4 py-3 font-medium text-gray-300">#{pkg.id}</td>
                  <td className="px-4 py-3 text-purple-400 font-mono text-xs">{pkg.dest_node}</td>
                  <td className="px-4 py-3">
                    <span className={`inline-flex items-center rounded-full px-2 py-0.5 text-xs font-medium ${
                      pkg.priority > 5 ? 'bg-red-500/10 text-red-400 border border-red-500/20' : 
                      pkg.priority > 2 ? 'bg-yellow-500/10 text-yellow-400 border border-yellow-500/20' : 
                      'bg-green-500/10 text-green-400 border border-green-500/20'
                    }`}>
                      Level {pkg.priority}
                    </span>
                  </td>
                  <td className="px-4 py-3 text-right tabular-nums text-gray-300">
                    {formatTime(pkg.deadline)}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
};

export default PendingQueue;
