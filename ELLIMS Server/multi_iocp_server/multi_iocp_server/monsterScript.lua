my_id = 9999
movestate = -4 --state 0 : fix, 1 : roaming peace, 2 : roaming aggro, 11 : aggro
initX = 0
initY = 0
name = ""
type = 0 --0 boss 1 2 3 4 monster
level = 1 --attack = level*3

function set_object_id (id, move, x, y, t, l, n) --나중에 파라미터로 안 받고 그냥 여기서 처리
	my_id = id
	movestate = move
	initX = x
	initY = y
	type = t
	level = l
	name = "monster"
end

function event_player_move(player_id)
	--여기서 roaming aggro 깨어남 처리
end

function event_player_attack(player_id)
	--peace는 여기서 깨어남 처리, aggro로
end

function npc_attack(player_id)
	--peace는 여기서 깨어남 처리, aggro로
end

function event_self_dead(player_id)
	--경험치 주고 타이머 세팅하는 함수 부르기
end

function event_self_revive(player_id)
	--return으로 initX initY
end

function npc_move(x, y, tx, ty)
	dir = 0

	if(movestate == 11) then
		dir = astar(x,y,tx,ty)
	elseif (movestate == 1 or movestate == 2) then
		dir = roam(x,y)
	end

	if (dir == 1) then
		if (x-1 > initX-20) then
			return x-1, y
		end
	end
	if (dir == 2) then
		if (x+1 < initX+20) then
			return x+1, y
		end
	end
	if (dir == 3) then
		if (y-1 > initY-20) then
			return x, y-1
		end
	end
	if (dir == 4) then
		if (y+1 < initY+20) then
			return x, y+1
		end
	end

	return x,y
end