#pragma once

#include <godot_cpp/classes/node.hpp>

namespace godot {

// M0 placeholder: proves the GDExtension loads and C++ runs inside the scene
// tree. From M1 this becomes the bridge that owns the fixed-step simulation
// loop (accumulator + interpolation) and exposes sim state to the renderer.
class SimCore : public Node {
	GDCLASS(SimCore, Node)

protected:
	static void _bind_methods();

public:
	void _ready() override;

	// Smoke-test hook callable from GDScript / the editor.
	String hello() const;
};

} // namespace godot
