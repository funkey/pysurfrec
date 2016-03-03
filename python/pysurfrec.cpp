#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <util/exceptions.h>
#include <surfrec/IlpSolver.h>
#include "logging.h"

template <typename Map, typename K, typename V>
const V& genericGetter(const Map& map, const K& k) { return map[k]; }
template <typename Map, typename K, typename V>
void genericSetter(Map& map, const K& k, const V& value) { map[k] = value; }

#if defined __clang__ && __clang_major__ < 6
// std::shared_ptr support
	template<class T> T* get_pointer(std::shared_ptr<T> p){ return p.get(); }
#endif

namespace surfrec {

/**
 * Translates an Exception into a python exception.
 *
 **/
void translateException(const Exception& e) {

	if (boost::get_error_info<error_message>(e))
		PyErr_SetString(PyExc_RuntimeError, boost::get_error_info<error_message>(e)->c_str());
	else
		PyErr_SetString(PyExc_RuntimeError, e.what());
}

/**
 * Defines all the python classes in the module libpymaxflow. Here we decide 
 * which functions and data members we wish to expose.
 */
BOOST_PYTHON_MODULE(surfrec) {

	boost::python::register_exception_translator<Exception>(&translateException);

	// Logging
	boost::python::enum_<logger::LogLevel>("LogLevel")
			.value("Quiet", logger::Quiet)
			.value("Error", logger::Error)
			.value("Debug", logger::Debug)
			.value("All", logger::All)
			.value("User", logger::User)
			;
	boost::python::def("setLogLevel", setLogLevel);
	boost::python::def("getLogLevel", getLogLevel);
			;

	// std::vector<double>
	boost::python::class_<std::vector<double>>("ColumnCosts")
			.def(boost::python::init<>())
			.def(boost::python::init<std::size_t>())
			.def(boost::python::vector_indexing_suite<std::vector<double>>())
			;

	// IlpSolver
	boost::python::class_<IlpSolver, boost::noncopyable>("IlpSolver", boost::python::init<std::size_t, std::size_t, int, int>())
			.def("add_nodes", &IlpSolver::add_nodes)
			.def("add_edge", &IlpSolver::add_edge)
			.def("set_level_costs", &IlpSolver::set_level_costs)
			.def("min_surface", &IlpSolver::min_surface)
			.def("level", &IlpSolver::level)
			.def("dump_ilp", &IlpSolver::dump_ilp)
			;

}

} // namespace surfrec
