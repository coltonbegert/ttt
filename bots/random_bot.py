from bots.base_bot import BaseBot

import random

class Bot(BaseBot):
    def request(self):
        return random.choice(self.board.get_valid())
