#!/bin/bash

# make player scores and ball positions
start_score=50
field_size=3
ball=0
points=( "$start_score" "$start_score" )


# board printing

: '
    player choice function
    @params : player_no, points the player has 
    @returns: choice, new amount of points 
'
function choose {
    local player_pick_str="PLAYER \$U PICK A NUMBER: "
    local invalid_move_str="NOT A VALID MOVE !"

    local choice=0 
    local points="$2"
    local player_no="$1"

    # keep prompting player until choice is in range and is a number 
    echo "${player_pick_str/\$U/${player_no}}"
    read  -s choice 

    while [[ ! "$choice" =~ ^[0-9]+$ ]] || \
          [[ "$choice" -gt "$points" ]] || \
          [[ "$choice" -le 0 ]]; do
        echo "$invalid_move_str" 
        echo "${player_pick_str/\$U/${player_no}}"
        read -s choice 
    done 
    
    return "$choice"
}

# check win

: '
    checks for a victory on the board 
    @params: ball position, player scores 
    @returns: 0 - no one, 1 - player 1, 2 - player 2, 3 - tie 
'
function check_win { 
        local won=0
        local p1="${points[0]}"
        local p2="${points[1]}"

        #out of bounds
        if [[ "$ball" -ge "$field_size" ]]; then 
            won=1          
        elif [[ "$ball" -le "-${field_size}" ]]; then 
            won=2

        # both have no score. settle the gagme 
        elif [[ "$p1" -le 0  &&  "$p2" -le 0 ]]; then 
            
            if [[ "$ball" -gt 0 ]]; then 
                won=1
            elif [[ "$ball" -lt 0 ]]; then 
                won=2
            else 
                won=3
            fi  

        # the ball is inbounds and only 1 has no score 
        elif [[ "$p1" -le 0 ]]; then 
            won=2
        elif [[ "$p2" -le 0 ]]; then 
            won=1
        fi
        return $won
}

: '
    prints victory based on winner 
    @params: winner. who won
    @returs: echos the winners winning message. 
             A tie if there is one  
'
function print_victory {
    local won_string="PLAYER \$U WINS !"
    local draw_string="IT'S A DRAW !"
    local winner="$1"

    if [[ "$winner" -eq 3 ]]; then 
        echo "$draw_string"
    else 
        echo "${won_string/\$U/${winner}}"
    fi
}

: '
    update the game variables after a turn 
    @params: ball position, player scores, player choices 
'
function update_board {
    local p1="$1" 
    local p2="$2" 
   
    # update player points 
    points[0]=$((${points[0]} - $p1))
    points[1]=$((${points[1]} - $p2))
   
    # check who won the turn and move ball
    if   [[ "$p1" -gt "$p2" ]]; then 
        if [[ "$ball" -lt 0 ]]; then 
            ball=1
        else 
            ball=$(("$ball" + 1))
        fi 

    elif [[ "$p1" -lt "$p2" ]]; then 
        if [[ "$ball" -gt 0 ]]; then 
            ball=-1
        else 
            ball=$(("$ball" - 1))
        fi 
    fi 

    check_win
    return $?
}


function print_board {
    local border=" --------------------------------- "
    local inside=" |       |       #       |       | "

    echo  " Player 1: ${points[0]}         Player 2: ${points[1]} "
    echo "$border"; echo "$inside"; echo "$inside"; 
    case "$ball" in
        3)
            echo " |       |       #       |       |O"
            ;;
        2) 
            echo " |       |       #       |   O   | "
            ;;
        1)
            echo " |       |       #   O   |       | "
            ;;
        0) 
            echo " |       |       O       |       | "
            ;;
        -1) echo " |       |   O   #       |       | "
            ;;
        -2) 
            echo " |   O   |       #       |       | "
            ;;
        -3)
            echo "O|       |       #       |       | "
            ;;
    esac
    echo "$inside"; echo "$inside"; echo "$border";


    if ! [[ "$1" -eq 0 || "$2" -eq 0 ]]; then   
        echo -e "       Player 1 played: ${1}\n       Player 2 played: ${2}\n\n"

    fi

}

: '
    game loop
        - print board
        - player choices 
        - update board 
        - check win

    announce victory
'
function play {

    # keep playing until someone wins 
    local end=0
    local c1=0
    local c2=0

    while [[ $end -eq 0 ]]; do 

        # print board state 
        print_board "$c1" "$c2" 

        # let players choose 
        choose 1 "${points[0]}"
        c1="$?"
        choose 2 "${points[1]}"
        c2="$?"

        # update board based on choices 
        update_board "$c1" "$c2"
        end=$?
        
    done
    print_board "$c1" "$c2"
    print_victory "$end"
}

play 

