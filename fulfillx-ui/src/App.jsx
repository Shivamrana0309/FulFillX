import React, { useState } from 'react'
import PendingQueue from './components/PendingQueue'
import DispatchPanel from './components/DispatchPanel'
import AddPackageForm from './components/AddPackageForm'

function App() {
  const [queueRefreshKey, setQueueRefreshKey] = useState(0);

  const handlePackageAdded = () => {
    // Increment the key to trigger a re-fetch in PendingQueue
    setQueueRefreshKey(prev => prev + 1);
  };

  return (
    <div className="min-h-screen bg-[#0f1115] text-gray-300 font-sans selection:bg-purple-500/30">
      
      {/* Header */}
      <header className="sticky top-0 z-50 flex items-center justify-between border-b border-gray-800 bg-[#161b22] px-6 py-4 shadow-sm">
        <div className="flex items-center space-x-3">
          <div className="flex h-8 w-8 items-center justify-center rounded-md bg-purple-600 text-white font-bold shadow-md shadow-purple-500/20">
            FX
          </div>
          <h1 className="text-xl font-semibold tracking-tight text-white">
            FulfillX <span className="text-gray-500 font-normal">| Delivery Routing Engine</span>
          </h1>
        </div>
        <div className="flex items-center space-x-4 text-sm">
          <span className="flex items-center space-x-2">
            <span className="relative flex h-2.5 w-2.5">
              <span className="absolute inline-flex h-full w-full animate-ping rounded-full bg-green-400 opacity-75"></span>
              <span className="relative inline-flex h-2.5 w-2.5 rounded-full bg-green-500"></span>
            </span>
            <span className="text-gray-400 font-medium">API Connected</span>
          </span>
        </div>
      </header>

      {/* Main Layout Grid */}
      <main className="mx-auto max-w-screen-2xl p-6">
        <div className="grid grid-cols-1 gap-6 lg:grid-cols-12">
          
          {/* Left Column: Pending Queue */}
          <section className="col-span-1 lg:col-span-4 rounded-xl border border-gray-800 bg-[#1c2128] shadow-lg overflow-hidden flex flex-col min-h-[600px]">
            <PendingQueue refreshKey={queueRefreshKey} />
          </section>

          {/* Right Column: Add Package & Dispatch Control */}
          <section className="col-span-1 lg:col-span-8 flex flex-col space-y-6">
            <div className="rounded-xl border border-gray-800 bg-[#1c2128] shadow-lg overflow-hidden shrink-0">
              <AddPackageForm onPackageAdded={handlePackageAdded} />
            </div>

            <div className="rounded-xl border border-gray-800 bg-[#1c2128] shadow-lg overflow-hidden flex-1 min-h-[400px]">
              <DispatchPanel />
            </div>
          </section>

        </div>
      </main>
    </div>
  )
}

export default App
