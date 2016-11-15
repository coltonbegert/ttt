import bots.human
import bots.random_bot
import bots.mctscomplex
import bots.mctspure

def get_bot(name):
    return {
        'human': human.Bot,
        'random': random_bot.Bot,
        'mcts': mctspure.Bot,
        'mctsplus': mctscomplex.Bot
    }[name]
