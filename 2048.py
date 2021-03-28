import random
import time
from colored import fg, bg, attr

class Grid:

    def __init__(self, rows, cols):
        self.rows = rows
        self.cols = cols
        self.grid = {}

    def add_tile(self, x, y, n):
        if (x,y) in self.grid:
            return
        self.grid[(x,y)] = n

    def add_random_tile(self):
        while True:
            x = random.randint(0, self.rows-1)
            y = random.randint(0, self.cols-1)
            n = random.randint(1, 2) * 2

            if (x,y) not in self.grid:
                self.add_tile(x,y,n)
                break

    # Push tile at position (x,y) into dx, dy direction
    # Merges two tiles if numbers match
    # returns True if state has changed.
    def push_tile(self, x, y, dx, dy):
        if (x,y) not in self.grid:
            return False
        if x+dx<0 or x+dx >= self.rows:
            return False
        if y+dy<0 or y+dy >= self.cols:
            return False

        # check if there is another tile in target position
        if (x+dx, y+dy) in self.grid:
            if self.grid[(x,y)] == self.grid[(x+dx,y+dy)]:
                self.grid[(x+dx,y+dy)] += self.grid[(x,y)]
                del self.grid[(x,y)]
                return True
        else:
                self.grid[(x+dx,y+dy)] = self.grid[(x,y)]
                del self.grid[(x,y)]
                return True
        return False
       
    def get_color(n):
        if n==2:
            return fg("#005500")
        if n==4:
            return fg("#006600")
        if n==8:
            return fg("#007700")
        if n==16:
            return fg("#008800")
        if n==32:
            return fg("#009900")
        if n==64:
            return fg("#00AA00")
        if n==128:
            return fg("#00BB00")
        if n==256:
            return fg("#00CC00")
        if n==512:
            return fg("#00DD00")
        if n==1024:
            return fg("#00EE00")
        return fg("#00FF00")
 
    def draw(self):
        # clear screen
        print("\033[H\033[J")
        for i in range(self.rows):
            row_str = ""
            for j in range(self.cols):
                if (i,j) in self.grid:
                    row_str += Grid.get_color(self.grid[(i,j)]) + ("%5d" % self.grid[(i,j)]) + attr(0)
                else:
                    row_str += ("     ")
            print(row_str)
        time.sleep(0.1)

    def down(self):
        board_changed = True
        while board_changed:
            board_changed = False
            for i in range(self.rows-1, -1, -1):
                for j in range(self.cols):
                    board_changed |= self.push_tile(i, j, +1, 0)
            grid.draw()

    def up(self):
        board_changed = True
        while board_changed:
            board_changed = False
            for i in range(self.rows):
                for j in range(self.cols):
                    board_changed |= self.push_tile(i, j, -1, 0)
            grid.draw()

    def left(self):
        board_changed = True
        while board_changed:
            board_changed = False
            for j in range(self.cols):
                for i in range(self.rows):
                    board_changed |= self.push_tile(i, j, 0, -1)
            grid.draw()

    def right(self):
        board_changed = True
        while board_changed:
            board_changed = False
            for j in range(self.cols-1, -1, -1):
                for i in range(self.rows):
                    board_changed |= self.push_tile(i, j, 0, +1)
            grid.draw()

grid = Grid(16, 8)

while True:
    grid.add_random_tile()
    grid.draw()

    direction = random.choice("hjkl")
    # direction = input("> ")
    if direction == "j":
        grid.down()
    if direction == "k":
        grid.up()
    if direction == "h":
        grid.left()
    if direction == "l":
        grid.right()

