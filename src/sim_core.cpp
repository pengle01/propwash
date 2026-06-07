#include "sim_core.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SimCore::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hello"), &SimCore::hello);
}

void SimCore::_ready() {
	UtilityFunctions::print("[Propwash] SimCore C++ extension loaded (M0).");
}

String SimCore::hello() const {
	return "Propwash sim core: hello from C++";
}
