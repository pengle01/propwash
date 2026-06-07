extends Node3D

# M0 smoke test: confirm the GDExtension class is registered and callable.
func _ready() -> void:
	var sim: Node = $SimCore
	print("[Propwash] GDScript sees SimCore: ", sim.hello())
