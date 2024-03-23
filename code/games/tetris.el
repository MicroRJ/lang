// NOTE:
// this is a crude tetris clone and it is probably the worst implementation ever since
// rotations are not even right!

// todo: draw following shapes
// todo: projections
// todo: reflections
// todo: wall kicks
// todo: put shapes in a subgrid for proper rotations

import "code/core/core.el"
import "code/core/lgi.el"

// If a piece has .5 seconds before locking down and the turn timer is .5 seconds,
// then there's no need to add any additional logic for checking whether a separate
// lock timer has expired, right...?
TURN_TIME ::= .5

Shape :: struct {
	colorid: int
	pt: int2*[4]*
}

make_I :: () -> Shape * {
	// [ ][*][ ][ ]
	pt := new int2*[] {
		new int2{ 0,0},
		new int2{-1,0},
		new int2{+1,0},
		new int2{+2,0},
	}
	ret new Shape { 2, pt }
}

make_L :: () -> Shape * {
	// [*][ ][ ]
	// [ ]
	pt := new int2*[4] {
		new int2{0, 0},
		new int2{1, 0},
		new int2{2, 0},
		new int2{0,-1},
	}
	ret new Shape { 3, pt }
}

make_T :: () -> Shape * {
	//  [ ][*][ ]
	//  	 [ ]
	pt := new int2*[4]
	pt[0] = new int2{0,0}
	pt[1] = new int2{-1,0}
	pt[2] = new int2{+1,0}
	pt[3] = new int2{0,-1}
	ret new Shape { 4, pt }
}

make_O :: () -> Shape * {
	//  [*][ ]
	//  [ ][ ]
	pt := new int2*[4]
	pt[0] = new int2{0,0}
	pt[1] = new int2{1,0}
	pt[2] = new int2{0,-1}
	pt[3] = new int2{1,-1}
	ret new Shape { 5, pt }
}

make_S :: () -> Shape * {
	//     [*][ ]
	//  [ ][ ]
	pt := new int2*[4]
	pt[0] = new int2{0,0}
	pt[1] = new int2{0,-1}
	pt[2] = new int2{1,0}
	pt[3] = new int2{-1,-1}
	ret new Shape { 6, pt }
}

Cell :: struct {
	colorid := 0
	stateid := 0
}

Game :: struct {
	// allocate more cells for when we spawn offscreen or if anything else
	// where to happen
	grid := new Cell *[512]
	grid_x: int
	grid_y: int
	cell_size: int
	next_shape: Shape *
	this_shape: Shape *

	position: int2 *
	// todo
	rotation: int2 *

	// 0 for the floor, 1 for the wall, the rest is ours
	thisid := 2

	FLOOR ::= 0
	WALL  ::= 1
	I ::= make_I()
	T ::= make_T()
	L ::= make_L()
	O ::= make_O()
	S ::= make_S()

	bag := new Shape*[5]
}

get2 :: (game: Game *, x: int, y: int) -> Cell * {
	ret game.grid[12 * y + x]
}

get :: (game: Game *, xy: int2 *) -> Cell * {
	ret game.grid[12 * xy.y + xy.x]
}

set2 :: (game: Game *, x: int, y: int, mystateid: int, mycolorid: int) -> int {
	cell := get2(game,x,y)
	cell.stateid = mystateid
	cell.colorid = mycolorid
	ret 1
}

set :: (game: Game *, xy: int2 *, mystateid: int, mycolorid: int) -> int {
	ret set2(game,xy.x,xy.y,mystateid,mycolorid)
}

vbeam :: (game: Game*, x: int, y: int, v: int, l := 22) -> int {
	for i in y..l ? set2(game,x,i,0,v)
	ret 1
}

hbeam :: (game: Game*, x: int, y: int, v: int, l := 12) -> int {
	for i in x..l ? set2(game,i,y,0,v)
	ret 1
}

// return true if found cell at row 'y'
hscan :: (game: Game *, y: int) -> int {
	for x in 1..11 ? {
		if get2(game,x,y).stateid != 0 ? {
			ret 1
		}
	}
	ret 0
}

hline :: (game: Game *, y: int) -> int {
	for x in 1..11 ? {
		cell ::= get2(game,x,y)
		if cell.stateid == 0 ? {
			ret 0
		}
	}
	ret 1
}

next :: (game: Game *) -> int {
	game.position.x = 5
	game.position.y = 21
	game.thisid = game.thisid + 1
	length ::= lengthOf(game.bag)
	i := rnd(0,length - 1)
	index ::= i
	game.this_shape = game.bag[index]
	ret 1
}

// 2d perp operator, left and right rotation
prp :: (v: int2 *, clockwise: int) -> int2 * {
	x: int
	y: int

	if clockwise == 1 ? {
		x = v.y
		y = 0 - v.x
	}

	if clockwise == 0 ? {
		x = 0 - v.y
		y = v.x
	}

	ret new int2{x,y}
}

// whether the current shape can place a point at xy
can :: (game: Game *, xy: int2 *) -> int {
	mystateid := game.thisid
	if 10 < xy.x ? {
		ret 0
	}
	if xy.x <= 0 ? {
		ret 0
	}
	if xy.y <= 0 ? {
		ret 0
	}
	cell ::= get(game,xy)
	if cell.stateid != mystateid ? {
		if cell.stateid != 0 ? {
			ret 0
		}
	}
	ret 1
}

plot :: (game: Game *, pts: int2*[1]*, stateid: int, colorid: int) -> int {
	p := game.position
	for i in 0..lengthOf(pts) ? set(game,int2_add(p,pts[i]),stateid,colorid)
	ret 1
}

clear_shape :: (game: Game *) -> int {
	ret plot(game,game.this_shape.pt,0,0)
}

redraw_shape :: (game: Game *) -> int {
	ret plot(game,game.this_shape.pt,game.thisid,game.this_shape.colorid)
}


move :: (game: Game *, dx: int, dy: int) -> int {
	pts := game.this_shape.pt
	ps := int2_add(game.position,new int2{dx,dy})

	for i in 0..lengthOf(pts) ? {
		if can(game,int2_add(ps,pts[i])) != true ? {
			ret 0
		}
	}
	clear_shape(game)
	game.position = ps
	redraw_shape(game)
	ret 1
}

// copy src row to dst row
rowcopy :: (game: Game *, dy: int, sy: int) -> int {
	for x in 1..11 ? {
		dc := get2(game,x,dy)
		sc := get2(game,x,sy)
		dc.colorid = sc.colorid
		dc.stateid = sc.stateid
	}
	ret 1
}

// shift down n items starting at y such that y.. = (y + n)..
vshft :: (game: Game *, y: int, n: int) -> int {
	c := 20 - y - n
	for i in 0..c ? {
		rowcopy(game,y+i,y+i+n)
	}
	ret 1
}

nline :: (game: Game *, y: int) -> int {
	n := 0
	for hline(game,y) ? {
		y = y + 1
		n = n + 1
	}
	ret n
}

// do this before the next shape is generated, otherwise it'll shift it too
line :: (game: Game *) -> int {
	for y in 1..20 ? {
		// is this a line
		is := hline(game,y)
		if is ? {
			// count number of lines after the one we found
			n := nline(game,y+1)
			vshft(game,y,n+1)
			y = y + n
		}
	}
	ret 0
}

rotate :: (game: Game *, clockwise: int) -> int {
	pts := game.this_shape.pt
	ps := game.position

	for i in 0..lengthOf(pts) ? {
		if can(game,int2_add(ps,prp(pts[i],clockwise))) != 1 ? {
			ret 0
		}
	}

	clear_shape(game)
	// cache the rotation
	for j in 0..lengthOf(pts) ? {
		pts[j] = prp(pts[j],clockwise)
	}
	redraw_shape(game)
	ret 1
}

clear :: (game: Game *) -> int {
	hbeam(game, 0, 0,1)
	vbeam(game, 0, 0,1)
	vbeam(game,11, 0,1)
	for n in 1..22 ? hbeam(game,1,n,0,11)
	ret 1
}

run_game :: () -> int {
	cell_size ::= 28

	G := new Game {}
	G.grid_x = 10
	G.grid_y = 20
	G.cell_size = cell_size
	G.position = new int2 { 5, 20-3 }

	G.bag[0] = G.T
	G.bag[1] = G.O
	G.bag[2] = G.L
	G.bag[3] = G.I
	G.bag[4] = G.S

	lgi_initAll(28*(12+2)+(16*2), 28*(22+2)+(16*2), "tetris")

	// rxresize(G.grid_x * cell_size + 16*2,G.grid_y * cell_size + 16*2);

	for k in 0..lengthOf(G.grid) ? G.grid[k] = new Cell {}
	grid ::= G.grid
	clear(G)
	next(G)

	no_error := 1

	// when the timer hits zero a new turn
	turn_timer := TURN_TIME


	itc ::= new float4 *[7]
	itc[0] = lgi.COLOR_BLACK
	itc[1] = lgi.COLOR_GRAY
	itc[2] = lgi.COLOR_CYAN
	itc[3] = lgi.COLOR_YELLOW
	itc[4] = lgi.COLOR_MAGENTA
	itc[5] = lgi.COLOR_ORANGE
	itc[6] = lgi.COLOR_GREEN


	game_over := 0
	for no_error ? {

		delta := lgi_time()
		turn_timer = turn_timer - delta

		size_x := lgi_getSizeX()
		size_y := lgi_getSizeY()

		if lgi_testKey('A') ? {
			move(G,-1,0)
		}
		if lgi_testKey('D') ? {
			move(G,+1,0)
		}
		if lgi_testKey('W') ? {
			rotate(G,true)
		}
		if lgi_testKey('S') ? {
			turn_timer = 0.
		}


		if turn_timer <= 0. ? {
			turn_timer = TURN_TIME
			could := move(G,0,-1)
			if could != true ? {
				// before the next shape is generated and plotted
				// game over!
				if hscan(G,20) ? {
					clear(G)
				}
				line(G)
				next(G)
			}
		}

		lgi_clearBackgroundColor(lgi.COLOR_BLACK)
		for y in 0..22  ? {
			for x in 0..12 ? {
				cell := get2(G,x,y)
				colorid := cell.colorid
				color := itc[colorid]
				if colorid != 0 ? {
					box_x := 16 + x * (cell_size+2)
					box_y := 16 + y * (cell_size+2)
					draw_box(box_x,box_y,cell_size,cell_size,3.,color)
				}
			}
		}
		no_error = lgi_tick()
	}

	ret 0
}

main :: () -> int {
	testelang()
	run_game()
	ret 0
}

