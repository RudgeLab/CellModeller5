#include <pybind11/pybind11.h>
#include <iostream>
#include <stdexcept>

#include "shader_compiler.h"
#include "simulator.h"

#ifndef CM_MODULE_NAME
#error A module name must be provided!
#endif

#ifdef CM_WITH_DEBUG_DIALOG
#ifdef WIN32
#include <Windows.h>

void handleDebugBehavior()
{
	DWORD pid = GetCurrentProcessId();
	std::string message = "Wait for debugger to attach? (PID: " + std::to_string(pid) + ")";

	DWORD action = MessageBoxA(NULL, message.c_str(), "CM5 Debugging", MB_YESNO | MB_SYSTEMMODAL);
	if (action != IDYES) return;

	int sleepFor = 200;
	int timeout = (int)(20 * (1000.0f / sleepFor));

	std::cout << "Waiting for debugger (PID: " << pid << ")\n";
	for (; timeout >= 0 && !IsDebuggerPresent(); timeout--) Sleep(sleepFor);

	if (timeout < 0) std::cout << "Debugger wait timeout\n";
	else std::cout << "Debugger detected\n";
}
#else
void handleDebugBehavior()
{
	//Do nothing
}
#endif
#endif

namespace py = pybind11;

class SimulatorInterface
{
private:
	Simulator* m_simulator = nullptr;
public:
	SimulatorInterface(const py::object& loadShaderCallback) :
		m_simulator(new Simulator())
	{
		auto importCallback = [&](const std::string& path)
		{
			return py::cast<std::string>(loadShaderCallback(path));
		};

#if defined(_DEBUG) || !defined(_NDEBUG)
		const bool withDebug = true;
#else
		const bool withDebug = false;
#endif

		CM_TRY_THROW_V(initSimulator(m_simulator, withDebug));
		CM_TRY_THROW_V(importShaders(*m_simulator, importCallback));
	}

	~SimulatorInterface()
	{
		deinitSimulator(*m_simulator);
	}

	void step()
	{
		CM_TRY_THROW_V(stepSimulator(*m_simulator));
	}

	double getLastStepTime()
	{
		return m_simulator->lastStepTime;
	}

	void dumpToStepFile(std::string filepath)
	{
		CM_TRY_THROW_V(writeSimulatorStateToStepFile(*m_simulator, filepath));
	}

	void dumpToVizFile(std::string filepath)
	{
		CM_TRY_THROW_V(writeSimulatorStateToVizFile(*m_simulator, filepath));
	}
};

PYBIND11_MODULE(CM_MODULE_NAME, m) {

#ifdef CM_WITH_DEBUG_DIALOG
	//Wait for a debugger to attach to the current process
	handleDebugBehavior();
#endif

	// Initialize the shader compiler
	if (!startupShaderCompiler())
	{
		throw std::runtime_error("Failed to initialize shader compiler");
	}

	//terminateShaderCompiler();

	py::class_<SimulatorInterface>(m, "NativeSimulator")
		.def(py::init<const py::object&>())
		// Since `step` might take a lot of time, we should release the GIL to allow other threads to run
		// while the step is being processed.
		.def("step", &SimulatorInterface::step, py::call_guard<py::gil_scoped_release>())
		.def("get_last_step_time", &SimulatorInterface::getLastStepTime)
		.def("dump_to_step_file", &SimulatorInterface::dumpToStepFile)
		.def("dump_to_viz_file", &SimulatorInterface::dumpToVizFile);
}