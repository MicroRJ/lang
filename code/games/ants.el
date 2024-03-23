import "code/core/core.el"
import "code/core/lgi.el"

// NOTE:
// this example simulates ants finding resources and returning them back home using
// only a few simple rules!
// the example is quite limited since elang struggles to process a large amount
// of ants and a high granularity pheromone grid!
// In release mode I've managed to run 512 ants with 16x16 resolution in 1920x1080 at
// at around 60fps.
// I could of-course optimize the program more but that's not really what I'm
// interested in at the moment!
// Hopefully in the future it'll work better and we can simulate many more!
// Especially once I add vector intrinsic!

// Problems with this simulation:
// A) Every other run the ants tend to clump up and struggle to find the path back home,
// I suspect this is because I haven't found the right balance of pheromone projection and
// decay over time, also ants are limited to a very low granularity pheromone map...Also
// they tend to prefer to stick to the trail too much, I've tried increasing their curiosity
// but I have to really just add a GUI to mess with the controls more conveniently.
// B) Ants do not head back:
// I didn't program the ants to return in the direction the came once they found the target
// and this leads to them preferring to wrap around the map.
// C) Sensors, at the moment the ants have 3 pheromone receptors orthogonal to each other
// at a random distance from the head but I would like to experiment with the degree of
// space between each and try only two antenna instead, more like real ants. This could
// encourage them to go off-trail more easily.
//
// Chances are the simulation will not really work as intended for you because it is
// fairly finicky... And every time something changes in elang I have to mess around
// with the parameters! My own damn fault!
//
BASE   ::= 0
RESOURCE ::= 1

DRAW_PHEROMONE_MAP ::= true
NUMBER_OF_ANTS ::= 128
PHEROMONE_MAP_RESOLUTION ::= 64
ANTENNA_LENGTH_TRAIT_MIN  ::= 64.
ANTENNA_LENGTH_TRAIT_MAX  ::= 128.
QUICKNESS_TRAIT_MIN ::= 32.
QUICKNESS_TRAIT_MAX ::= 64.
PHEROMONE_STEER_CORRELATION_FACTOR ::= 1.
PHEROMONE_EMITION_PER_SECOND ::= 12.5
PHEROMONE_DECAY_PER_SECOND   ::= 1.25

Ant :: struct {
	target := RESOURCE
	position: float2
	velocity: float2
	direction: float2 = rnd_dir()
	antenna_separation: float2
	pheromone_steering_force: float2
	curiosity_steering_force: float2
	quickness_trait ::= iton(rnd(QUICKNESS_TRAIT_MIN,QUICKNESS_TRAIT_MAX))
	curiosity_timer := .0
	antenna_length_trait ::= mix(ANTENNA_LENGTH_TRAIT_MIN,ANTENNA_LENGTH_TRAIT_MAX,rnd_n())
	antenna_timer := .0
	pheromone_timer := .0
}

ants: Ant *[]*

pheromone_texture_food: float [] *
pheromone_texture_home: float [] *
texture_resolution ::= PHEROMONE_MAP_RESOLUTION
texture_width: int
texture_height: int

resource_locations: float2 *[] *
number_resource_locations: int
home_location: float2 *

view_size_x: float
view_size_y: float

sample :: (p: float2, k: int) -> float {
	x ::= ntoi(p.x / iton(texture_resolution))
	y ::= ntoi(p.y / iton(texture_resolution))
	// __log(__fmt("x,y := %,%",x,y))
	if x < 0 ? {
		x = x + texture_width
	}
	if y < 0 ? {
		y = y + texture_height
	}
	x = x % texture_width
	y = y % texture_height
	i ::= texture_width * y + x
	result := 0.
	if k == BASE ? {
		result = pheromone_texture_home[i]
	}
	if k == RESOURCE ? {
		result = pheromone_texture_food[i]
	}
	ret sat(result / iton(texture_resolution),0.,1.)
}

trail :: (p: float2, food: float, home: float) -> int {
	x ::= ntoi(p.x / iton(texture_resolution))
	y ::= ntoi(p.y / iton(texture_resolution))
	// __log(__fmt("x,y := %,%",x,y))
	if x < 0 ? {
		x = x + texture_width
	}
	if y < 0 ? {
		y = y + texture_height
	}
	x = x % texture_width
	y = y % texture_height
	i ::= texture_width * y + x

	pheromone_texture_food[i] = pheromone_texture_food[i] + food
	pheromone_texture_home[i] = pheromone_texture_home[i] + home
	pheromone_texture_food[i] = sat(pheromone_texture_food[i],0,texture_resolution)
	pheromone_texture_home[i] = sat(pheromone_texture_home[i],0,texture_resolution)
	ret 0
}


paint :: (p: float2, food: float, home: float, radius: float) -> int {
	ret trail(local float2{p.x,p.y},food,home)
}


make_random_direction :: (reference: float2) -> float2 {
	range ::= PI / 2.
	ret rnd_dir(reference,-range,+range)

	result := local float2 {}
	metric := - 1.
	for i in 0..4 ? {
		test ::= float2_random_normal()
		dp ::= dot(reference,test)
		if metric <= dp ? {
			metric = dp
			result = test
		}
	}
	ret result
}

move_ant :: (xx: Ant *, delta: float) -> int {

	xx.curiosity_timer = xx.curiosity_timer - delta
	if xx.curiosity_timer <= .0 ? {
		xx.curiosity_timer = mix(.01,.2,rnd_n())
		impulse ::= .8
		xx.curiosity_steering_force = float2_scale(make_random_direction(xx.direction),impulse)
	}

	xx.antenna_timer = xx.antenna_timer - delta
	if xx.antenna_timer <= 0. ? {
		xx.antenna_timer = .15
		forward ::= xx.direction
		right ::= cross(forward)
		left ::= negate(right)

		right = float2_lerp(right,forward,xx.antenna_separation.x)
		left = float2_lerp(left,forward,xx.antenna_separation.y)

		antenna_length_trait ::= xx.antenna_length_trait

		forward_antenna ::= float2_add(xx.position,float2_scale(forward,antenna_length_trait))
		forward_antenna_input ::= sample(forward_antenna,xx.target)

		right_antenna ::= float2_add(xx.position,float2_scale(right,antenna_length_trait))
		right_antenna_input ::= sample(right_antenna,xx.target)

		left_antenna ::= float2_add(xx.position,float2_scale(left,antenna_length_trait))
		left_antenna_input ::= sample(left_antenna,xx.target)

		// basic circuit which simulates how an ant would steer based on its
		// pheromone receptors.
		steer_direction := forward
		antenna_input := forward_antenna_input

		if antenna_input < left_antenna_input ? {
			antenna_input = left_antenna_input
			steer_direction = left
		}
		if antenna_input < right_antenna_input ? {
			antenna_input = right_antenna_input
			steer_direction = right
		}

		xx.pheromone_steering_force = float2_scale(steer_direction,PHEROMONE_STEER_CORRELATION_FACTOR)
	}


	// TODO:
	// only do this when the force is updated!
	steer_force ::= float2_add(xx.curiosity_steering_force,xx.pheromone_steering_force)
	full_velocity_towards ::= float2_scale(steer_force,xx.quickness_trait)
	acceleration ::= float2_sub(full_velocity_towards,xx.velocity)
	xx.velocity = float2_add(xx.velocity,float2_scale(acceleration,delta))
	xx.velocity = float2_scale(xx.velocity,.8)
	xx.velocity = float2_clamp_to_length(xx.velocity,256.)

	// TODO
	if float2_length(xx.velocity) != 0. ? {
		xx.direction = normalize(xx.velocity)
	}

	xx.position = float2_add(xx.position,xx.velocity)
	if xx.position.x < 0. ? {
		xx.position.x = view_size_x - 1.
	}
	if xx.position.y < 0. ? {
		xx.position.y = view_size_y - 1.
	}
	if view_size_x <= xx.position.x ? {
		xx.position.x = 0.
	}
	if view_size_y <= xx.position.y ? {
		xx.position.y = 0.
	}

	// TODO:
	// could add the resource to a separate resource map for faster lookups!
	for i in 0..number_resource_locations ? {
		location ::= resource_locations[i]
		towards ::= float2_sub(location,xx.position)
		if float2_length(towards) < 32. ? {
			xx.target = BASE
			// TODO: break!
			// TODO: proper collision!
			// backwards ::= reflect(xx.direction,negate(normalize(towards)))
			// xx.velocity = float2_add(xx.velocity,float2_scale(backwards,delta))
		}
	}

	{
		towards ::= float2_sub(home_location,xx.position)
		if float2_length(towards) < 32. ? {
			xx.target = RESOURCE
			// TODO: break!
			// TODO: proper collision!
			// backwards ::= reflect(xx.direction,negate(normalize(towards)))
			// xx.velocity = float2_add(xx.velocity,float2_scale(backwards,delta))
		}
	}


	xx.pheromone_timer = xx.pheromone_timer - delta
	if xx.pheromone_timer <= 0. ? {
		xx.pheromone_timer = .01
		if xx.target == RESOURCE ? {
			trail(xx.position,0.,PHEROMONE_EMITION_PER_SECOND * delta)
		}
		if xx.target ==     BASE ? {
			trail(xx.position,PHEROMONE_EMITION_PER_SECOND * delta,0.)
		}
	}
	ret 0
}

draw_ant :: (xx: Ant *) -> int {
	r ::= local float2{xx.position.x,xx.position.y}
	d ::= negate(xx.direction)

	t ::= 2. * 2.
	b ::= 4. * 2.
	h ::= 3. * 2.

	// draw the head
	draw_circle(r.x,r.y,h,lgi.COLOR_ORANGE)
	r.x = r.x + (h + 1.) * d.x
	r.y = r.y + (h + 1.) * d.y

	if true ? {
		// torso (should be at 0.)
		r.x = r.x + (t + 1.) * d.x
		r.y = r.y + (t + 1.) * d.y
		if xx.target == BASE ? {
			draw_circle(r.x,r.y,t,lgi.COLOR_ORANGE)
		}
		if xx.target == RESOURCE ? {
			draw_circle(r.x,r.y,t,lgi.COLOR_MAGENTA)
		}
		r.x = r.x + t * d.x
		r.y = r.y + t * d.y

		// draw the butt
		r.x = r.x + b * d.x
		r.y = r.y + b * d.y
		draw_circle(r.x,r.y,b,lgi.COLOR_ORANGE)

		forward ::= xx.direction
		right ::= cross(forward)
		left ::= negate(right)
		antenna_length_trait ::= xx.antenna_length_trait
		forward_antenna ::= float2_add(xx.position,float2_scale(forward,antenna_length_trait))
		right_antenna ::= float2_add(xx.position,float2_scale(right,antenna_length_trait))
		left_antenna ::= float2_add(xx.position,float2_scale(left,antenna_length_trait))

		forward_antenna_input ::= sample(forward_antenna,xx.target)
		right_antenna_input ::= sample(right_antenna,xx.target)
		left_antenna_input ::= sample(left_antenna,xx.target)

		color ::= lgi.COLOR_CYAN
		color.w = .1 + forward_antenna_input
		draw_circle(forward_antenna.x,forward_antenna.y,h*.2,color)

		color.w = .1 + right_antenna_input
		draw_circle(right_antenna.x,right_antenna.y,h*.2,color)

		color.w = .1 + left_antenna_input
		draw_circle(left_antenna.x,left_antenna.y,h*.2,color)

		// basic circuit which simulates how an ant would steer based on its
		// pheromone receptors.
		steer_direction := forward_antenna
		antenna_input := forward_antenna_input
		antenna := forward_antenna
		if antenna_input < left_antenna_input ? {
			antenna_input = left_antenna_input
			steer_direction = left
			antenna = left_antenna
		}
		if antenna_input < right_antenna_input ? {
			antenna_input = right_antenna_input
			steer_direction = right
			antenna = right_antenna
		}

		draw_circle(antenna.x,antenna.y,h*.2,lgi.COLOR_MAGENTA)
	}

	ret 0
}

run_game :: () -> int {
	lgi_initAll(1980,1080,"ants")
	size_x ::= iton(lgi_getSizeX())
	size_y ::= iton(lgi_getSizeY())
	view_size_x = size_x
	view_size_y = size_y

	ants = new Ant *[NUMBER_OF_ANTS]
	texture_width = ntoi(size_x) / texture_resolution
	texture_height = ntoi(size_y) / texture_resolution
	pheromone_texture_food = new float[texture_width * texture_height]
	pheromone_texture_home = new float[texture_width * texture_height]

	number_resource_locations = 1
	resource_locations = new float2 *[100]
	resource_locations[0] = new float2 { size_x * .7, size_y * .7 }
	home_location = new float2 {size_x * .2, size_y * .2}

	for i in 0..lengthOf(ants) ? {
		ants[i] = new Ant {}
		// ants[i].position  = new float2{iton(rnd(0,size_x)),iton(rnd(0,size_y))}
		ants[i].position = local float2{home_location.x,home_location.y}
	}

	base ::= local float2{iton(rnd(0,size_x)),iton(rnd(0,size_y))}
	target ::= local float2{iton(rnd(0,size_x)),iton(rnd(0,size_y))}

	is_button_down := 0
	no_error := 1
	for no_error ? {
		delta := lgi_time()
		cursor_x ::= iton(lgi_getCursorX())
		cursor_y ::= iton(lgi_getCursorY())
		cursor ::= local float2{cursor_x,cursor_y}

		lgi_clearBackgroundColor(lgi.COLOR_BLACK)


		if lgi_testButton(0) ? {
			number_resource_locations = 0
			resource_locations[number_resource_locations] = new float2{}
			resource_locations[number_resource_locations].x = cursor_x
			resource_locations[number_resource_locations].y = cursor_y
			number_resource_locations = number_resource_locations + 1
		}

		if is_button_down ? {
			paint(cursor,texture_resolution,texture_resolution,32.)
		}


		for i in 0..lengthOf(pheromone_texture_food) ? {
			x ::= (i % texture_width) * texture_resolution
			y ::= (i / texture_width) * texture_resolution
			food ::= pheromone_texture_food[i]
			home ::= pheromone_texture_home[i]

			if DRAW_PHEROMONE_MAP ? {
				alpha ::= sqrt(food*food + home*home) / sqrt((iton(texture_resolution)*iton(texture_resolution))*2.)
				color ::= local float4{food,home,0.,alpha}
				draw_box(x,y,texture_resolution,texture_resolution,0.,color)
			}

			if 0. < food ? {
				pheromone_texture_food[i] = food - (PHEROMONE_DECAY_PER_SECOND * delta)
			}
			if 0. < home ? {
				pheromone_texture_home[i] = home - (PHEROMONE_DECAY_PER_SECOND * delta)
			}
		}

		for i in 0..lengthOf(ants) ? {
			xx ::= ants[i]
			move_ant(xx,delta)
			draw_ant(xx)
		}

		for i in 0..number_resource_locations ? {
			xx ::= resource_locations[i]
			draw_circle(xx.x,xx.y,8.,lgi.COLOR_GREEN)
		}
		draw_circle(home_location.x,home_location.y,8.,lgi.COLOR_RED)

		no_error = lgi_tick()
	}

	ret 0
}

main :: () -> int {
	.0
	run_game()
	ret 0
}

