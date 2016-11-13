import bots.human
import bots.random_bot

def get_bot(name):
    return {
        'human': human.Bot,
        'random': random_bot.Bot
    }[name]
