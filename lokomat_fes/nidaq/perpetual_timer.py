from threading import Timer


class PerpetualTimer:
    def __init__(self, t, hFunction):
        self.t = t
        self.hFunction = hFunction
        self.thread = Timer(self.t, self.handle_function)

    def handle_function(self):
        self.hFunction()
        self.thread = Timer(self.t, self.handle_function)
        try:
            self.thread.start()
        except RuntimeError:
            # This should not happen, but it does when the user is in debug mode and blocked on a breakpoint
            pass

    def start(self):
        self.thread.start()

    def cancel(self):
        self.thread.cancel()
