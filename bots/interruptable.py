from bots.base_bot import BaseBot
import bots
from time import sleep

class Bot(BaseBot):
    def setup(self, *args):
        self.bot = bots.get_bot(args[0])(self.board.clone(), self.player)
        self.human = bots.get_bot("human")(self.board.clone(), self.player)

        self.interrupted = False

    def start(self):
        self.bot.start()
        self.human.start()

    def stop(self):
        self.bot.stop()
        self.human.stop()

    def update(self, last_player, last_move):
        super(Bot, self).update(last_player, last_move)
        self.bot.update(last_player, last_move)
        self.human.update(last_player, last_move)

    def request(self):
        while True:
            if not self.interrupted:
                try:
                    print("Press Ctrl+C to interrupt in the next 3 seconds...")
                    sleep(3)
                    print("Letting bot take control.")
                    return self.bot.request()
                except KeyboardInterrupt:
                    print("\nGiving control to human.")
                    self.interrupted = True
            try:
                return self.human.request()
            except KeyboardInterrupt:
                self.interrupted = False
                print("\nRestoring control to bot...")
