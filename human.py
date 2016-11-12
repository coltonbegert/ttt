"""
Fake bot for using Human intelligence.
"""
import threading
import time
import sys
import select

class Human:
    def __init__(self, board):
        self._board = board.clone()

    def start(self):
        pass

    def predict(self):
        print("Player", self._board.player)

        def run():
            while True:
                B = self._board.clone()
                print(B)
                if B._next_board is None:
                    br = get_input("Board row: ")
                    bc = get_input("Board col: ")
                else:
                    br, bc = B._next_board

                print("Playing in board {}, {}".format(br, bc))

                mr = get_input("Row: ")
                mc = get_input("Col: ")

                r = br*3 + mr
                c = bc*3 + mc

                try:
                    B.move(r,c, B.player)
                except AssertionError as e:
                    print("##############")
                    print(e)
                    print("##############")
                    continue

                self.move = (r,c)
                return
        t = threading.Thread(target=run, daemon=True)
        t.start()
        t.join()
        return self.move

    def update(self, move):
        self._board.move(*move)

# Use a GUI to prompt for input since otherwise threading 'input()' fails
# SOURCE: http://stackoverflow.com/questions/15522336/text-input-in-tkinter

# TODO: Show the board in the prompt

from tkinter import *

class takeInput(object):
    def __init__(self,requestMessage):
        self.root = Tk()
        self.string = ''
        self.frame = Frame(self.root)
        self.frame.pack()
        self.acceptInput(requestMessage)

    def acceptInput(self,requestMessage):
        r = self.frame

        k = Label(r,text=requestMessage)
        k.pack(side='left')
        self.e = Entry(r,text='Name')
        self.e.pack(side='left')
        self.e.focus_set()
        b = Button(r,text='okay',command=self.gettext)
        b.pack(side='right')

    def gettext(self):
        self.string = self.e.get()
        self.root.destroy()

    def getString(self):
        return self.string

    def waitForInput(self):
        self.root.mainloop()

def getText(requestMessage):
    msgBox = takeInput(requestMessage)
    #loop until the user makes a decision and the window is destroyed
    msgBox.waitForInput()
    return msgBox.getString()

def get_input(prompt):
    while True:
        try:
            return int(getText(prompt))
        except ValueError:
            print("##############")
            print("Invalid input")
            print("##############")

