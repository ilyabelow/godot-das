require boost/godot
require Main


[godot_class(Node2D)]
class Pipes

	velocity = Vector2(-200, 0)
	game : Main?
	stopped : bool = false

	def _ready()
		game = native |> get_parent() |> get_parent() as Main

	def stop()
		stopped = true

	def _process(dt : float)
		if stopped
			return

		let prev_pos = native |> get_position()

		native |> translate(velocity * dt)

		let pos = native |> get_position()
		if prev_pos.x > game->get_bird_level() && pos.x < game->get_bird_level()
			game->add_point()

		if pos.x < - pipe_width * 0.5
			native |> queue_free()
