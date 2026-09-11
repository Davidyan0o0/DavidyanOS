import System
import traceback

RESULT = r"C:\Users\ADMINI~1\AppData\Local\Temp\kilo\mini8_test.result"
System.IO.File.WriteAllText(RESULT, "START")

def settle(tester):
    if not tester.Evaluate():
        raise Exception("MINI8 circuit oscillated")


def finish(code):
    System.Environment.ExitCode = code
    System.Environment.Exit(code)


try:
    from LogicCircuit import App

    phase = "CREATE_TESTER"
    tester = App.CreateTester("MINI8 CPU")
    phase = "RESET"
    tester.SetInput("CLK", 0)
    tester.SetInput("RESET", 1)
    settle(tester)
    tester.SetInput("CLK", 1)
    settle(tester)
    tester.SetInput("CLK", 0)
    settle(tester)
    tester.SetInput("RESET", 0)
    settle(tester)

    for cycle in range(40):
        phase = "CYCLE %d" % cycle
        tester.SetInput("CLK", 1)
        settle(tester)
        tester.SetInput("CLK", 0)
        settle(tester)
        if tester.GetOutput("HALT") == 1:
            break

    actual = {
        "A": tester.GetOutput("A"),
        "MEM_F": tester.GetOutput("MEM_F"),
        "HALT": tester.GetOutput("HALT"),
    }
    expected = {"A": 5, "MEM_F": 5, "HALT": 1}
    if actual != expected:
        raise Exception("MINI8 assertion failed: expected %s, got %s" % (expected, actual))
    print("MINI8 PASS cycles=%d A=%d MEM_F=%d HALT=%d" %
          (cycle + 1, actual["A"], actual["MEM_F"], actual["HALT"]))
    System.IO.File.WriteAllText(RESULT, "PASS cycles=%d A=%d MEM_F=%d HALT=%d" %
                               (cycle + 1, actual["A"], actual["MEM_F"], actual["HALT"]))
    finish(0)
except Exception as error:
    if hasattr(error, "clsException"):
        rendered = error.clsException.ToString()
    else:
        rendered = error.ToString() if hasattr(error, "ToString") else repr(error)
    detail = "%s %s: %s\n%s" % (
        phase, type(error).__name__, rendered, traceback.format_exc())
    print("MINI8 FAIL: %s" % detail)
    System.IO.File.WriteAllText(RESULT, "FAIL %s" % detail)
    finish(2)
