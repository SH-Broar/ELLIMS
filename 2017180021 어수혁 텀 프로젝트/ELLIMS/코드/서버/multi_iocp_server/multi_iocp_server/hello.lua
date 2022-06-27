my_id = 9999
movestate = -4 --state 0 : roaming, 1~ : movement
randDirection = -5

function set_object_id (id)
	my_id = id
	movestate = 0
	randDirection = -1
end

function event_player_move(player_id)
	if (movestate <= 0) then
		player_x = API_get_x(player_id)
		player_y = API_get_y(player_id)
		my_x = API_get_x(my_id)
		my_y = API_get_y(my_id)
		if (player_x == my_x) then
			if (player_y == my_y) then
				API_chat(player_id, my_id, "HELLO");
				movestate = 3
				randDirection = math.random(1,3)
			end
		end
	end
end

function npc_moved()
	if movestate > 0 then
		movestate = movestate - 1
		if movestate <= 0 then
			movestate = 0
			randDirection = -1
			API_chat(player_id, my_id, "BYE");
		end
	end
end