import React, { useState } from 'react';

const AddPackageForm = ({ onPackageAdded }) => {
  const [formData, setFormData] = useState({
    id: '',
    dest_node: '',
    priority: '',
    deadline: ''
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [success, setSuccess] = useState(false);

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
    setSuccess(false);

    if (!formData.id || !formData.dest_node || !formData.priority || !formData.deadline) {
      setError('All fields are required.');
      setLoading(false);
      return;
    }

    try {
      const response = await fetch('http://localhost:8080/api/package', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          id: parseInt(formData.id, 10),
          dest_node: parseInt(formData.dest_node, 10),
          priority: parseInt(formData.priority, 10),
          deadline: parseInt(formData.deadline, 10)
        })
      });

      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.message || data.error || `HTTP error! status: ${response.status}`);
      }

      setSuccess(true);
      setFormData({
        id: '',
        dest_node: '',
        priority: '',
        deadline: ''
      });

      if (onPackageAdded) {
        onPackageAdded();
      }

      // Hide success message after 3 seconds
      setTimeout(() => setSuccess(false), 3000);

    } catch (err) {
      setError(err.message || 'Failed to add package. Ensure the engine is running.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="flex flex-col bg-[#1c2128]">
      <div className="border-b border-gray-800 bg-[#22272e] px-5 py-3 flex justify-between items-center">
        <h2 className="text-sm font-semibold text-white uppercase tracking-wider">Add New Package</h2>
      </div>

      <div className="p-6 flex flex-col space-y-4">
        
        {error && (
          <div className="flex items-center space-x-3 rounded-lg border border-red-900/50 bg-red-500/10 p-3 text-red-400 shadow-sm">
            <svg className="h-5 w-5 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
            </svg>
            <p className="text-sm font-medium">{error}</p>
          </div>
        )}

        {success && (
          <div className="flex items-center space-x-3 rounded-lg border border-green-900/50 bg-green-500/10 p-3 text-green-400 shadow-sm">
            <svg className="h-5 w-5 shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M5 13l4 4L19 7" />
            </svg>
            <p className="text-sm font-medium">Package added successfully!</p>
          </div>
        )}

        <form onSubmit={handleSubmit} className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          <div>
            <label className="mb-1 block text-xs font-medium text-gray-400">Package ID</label>
            <input
              type="number"
              name="id"
              value={formData.id}
              onChange={handleInputChange}
              placeholder="e.g. 1"
              className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
            />
          </div>
          <div>
            <label className="mb-1 block text-xs font-medium text-gray-400">Dest. Node (1-5)</label>
            <input
              type="number"
              name="dest_node"
              value={formData.dest_node}
              onChange={handleInputChange}
              placeholder="e.g. 5"
              min="1"
              max="5"
              className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
            />
          </div>
          <div>
            <label className="mb-1 block text-xs font-medium text-gray-400">Priority (1-5)</label>
            <input
              type="number"
              name="priority"
              value={formData.priority}
              onChange={handleInputChange}
              placeholder="e.g. 3"
              min="1"
              max="5"
              className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
            />
          </div>
          <div>
            <label className="mb-1 block text-xs font-medium text-gray-400">Deadline (Unix)</label>
            <input
              type="number"
              name="deadline"
              value={formData.deadline}
              onChange={handleInputChange}
              placeholder="e.g. 1700005000"
              className="w-full rounded border border-gray-700 bg-[#161b22] px-3 py-2 text-sm text-gray-200 placeholder-gray-600 focus:border-purple-500 focus:outline-none focus:ring-1 focus:ring-purple-500 transition-colors"
            />
          </div>
          <div className="md:col-span-2 lg:col-span-4 mt-2">
            <button
              type="submit"
              disabled={loading}
              className="w-full flex items-center justify-center space-x-2 rounded bg-green-600 px-4 py-2.5 text-sm font-bold text-white shadow-md shadow-green-500/20 hover:bg-green-500 transition-colors disabled:opacity-50"
            >
              {loading && (
                <svg className="h-4 w-4 animate-spin" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
                </svg>
              )}
              <span>+ Add Package to Engine</span>
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default AddPackageForm;
