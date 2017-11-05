import random

coords = [(i+1, j+1) for i in range(60) for j in range(60)]
delta = [(i, j) for i in range(-1, 2) for j in range(-1, 2) if i!=0 or j!=0]

def generate_board():
    board = [[0]*62 for i in range(62)]
    res_list = [1]*1400+[2]*1400+[0]*(60*60-2800)
    random.shuffle(res_list)

    for (x, y), res in zip(coords, res_list):
        board[x][y] = res

    return board

def calc_content(board, x_, y_):
    neighbor = [board[x_+i][y_+j] for (i, j) in delta]
    empty_neighbor_cnt = neighbor.count(0)
    
    if empty_neighbor_cnt == len(neighbor):
        return 1.
    else:
        return neighbor.count(board[x_][y_]) / (len(neighbor) - empty_neighbor_cnt)
        
def iterate(board, threshold):
    old_pos = list(filter(lambda p: calc_content(board, p[0], p[1]) < threshold, coords))
    new_pos = old_pos.copy()
    random.shuffle(new_pos)

    board_copy = [row.copy() for row in board]
    for (x_, y_), (x, y) in zip(old_pos, new_pos):
        board_copy[x_][y_] = board[x][y]

    return board_copy

board = generate_board()
for x in range(20):
    board = iterate(board, 0.6)

s = ["  ", "--", "||"]
for x, y in coords:
    print(s[board[x][y]], end='')
    if(y == 1): print()