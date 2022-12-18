import { Worklets } from "react-native-worklets";
import { Expect, ExpectException, ExpectValue } from "./utils";

export const worklet_context_tests = {
  call_sync_on_js_thread: () => {
    const worklet = (a: number) => {
      "worklet";
      return a;
    };
    return ExpectValue(worklet(100), 100);
  },

  call_sync_on_js_thread_with_error: () => {
    const worklet = () => {
      "worklet";
      throw new Error("Test error");
    };
    return ExpectException(worklet, "Test error");
  },

  call_async_to_worklet_thread: () => {
    const x = 100;
    const f = (a: number) => {
      "worklet";
      return a + x;
    };
    const w = Worklets.createRunInContextFn(f);
    return ExpectValue(w(100), 200);
  },

  call_async_to_worklet_thread_context: () => {
    const context = Worklets.createContext("test");
    const f = (a: number) => {
      "worklet";
      return a;
    };
    const w = Worklets.createRunInContextFn(f, context);
    return ExpectValue(w(100), 100);
  },

  call_async_to_worklet_thread_with_error: () => {
    const f = () => {
      "worklet";
      throw new Error("Test error");
    };
    const w = Worklets.createRunInContextFn(f);
    return ExpectException(w, "Test error");
  },

  call_async_to_worklet_thread_with_error_in_context: () => {
    const context = Worklets.createContext("test");
    const f = () => {
      "worklet";
      throw new Error("Test error");
    };
    const w = Worklets.createRunInContextFn(f, context);
    return ExpectException(w, "Test error");
  },

  call_async_to_worklet_thread_and_call_second_worklet: () => {
    const sharedValue = Worklets.createSharedValue(0);
    const workletB = function (a: number) {
      "worklet";
      sharedValue.value = a;
    };

    const workletA = function (a: number) {
      "worklet";
      workletB(a);
    };

    const w = Worklets.createRunInContextFn(workletA);
    return Expect(w(100), () => {
      return sharedValue.value === 100
        ? undefined
        : `sharedValue.value to be 100, got ${sharedValue.value}`;
    });
  },

  call_sync_to_js_from_worklet: () => {
    const sharedValue = Worklets.createSharedValue(0);
    const setSharedValue = function (a: number) {
      "worklet";
      sharedValue.value = a;
    };

    const js1 = Worklets.createRunInJsFn(setSharedValue);

    const w1 = function (a: number) {
      "worklet";
      js1(a);
    };

    const w = Worklets.createRunInContextFn(w1);
    return Expect(w(100), () => {
      return sharedValue.value === 100
        ? undefined
        : `sharedValue.value to be 100, got ${sharedValue.value}`;
    });
  },

  call_sync_to_js_from_worklet_with_retval: () => {
    const workletB = Worklets.createRunInJsFn(function (a: number) {
      "worklet";
      return a;
    });

    const workletA = Worklets.createRunInContextFn(function (a: number) {
      "worklet";
      return workletB(a);
    });
    return ExpectValue(workletA(200), 200);
  },

  call_sync_to_js_from_worklet_with_error: () => {
    const callback = Worklets.createRunInJsFn(() => {
      "worklet";
      throw new Error("Test error");
    });

    const workletA = Worklets.createRunInContextFn(function () {
      "worklet";
      callback();
    });
    return ExpectException(workletA, "Exception in HostFunction: Test error");
  },

  call_decorated_js_function_from_worklet: () => {
    const adder = (a: number) => {
      "worklet";
      return a + a;
    };

    const w_square = Worklets.createRunInContextFn(function (a: number) {
      "worklet";
      return Math.sqrt(adder(a));
    });

    return ExpectValue(w_square(32), 8);
  },

  call_async_to_and_from_worklet: () => {
    const sharedValue = Worklets.createSharedValue(0);
    const workletB = function (b: number) {
      "worklet";
      sharedValue.value = b;
    };

    const workletA = Worklets.createRunInContextFn(function (a: number) {
      "worklet";
      return workletB(a);
    });

    return Expect(workletA(100), () => {
      return sharedValue.value === 100
        ? undefined
        : `sharedValue.value to be 100, got ${sharedValue.value}`;
    });
  },

  call_async_to_and_from_worklet_with_return_value: () => {
    const workletB = () => {
      "worklet";
      return 1000;
    };
    const workletA = Worklets.createRunInContextFn(function () {
      "worklet";
      return workletB();
    });
    return ExpectValue(workletA(), 1000);
  },

  call_async_to_and_from_worklet_multiple_times_with_return_value: () => {
    const workletB = (): number => {
      "worklet";
      return 1000;
    };
    const workletA = Worklets.createRunInContextFn(function () {
      "worklet";
      let r = 0;
      for (let i = 0; i < 100; i++) {
        r += workletB();
      }
      return r;
    });
    return ExpectValue(workletA(), 100000);
  },

  call_async_to_and_from_worklet_with_error: () => {
    const workletB = () => {
      "worklet";
      throw Error("Test error");
    };
    const workletA = Worklets.createRunInContextFn(function () {
      "worklet";
      return workletB();
    });
    return ExpectException(workletA, "Test error");
  },

  call_worklet_to_worklet_without_wrapping_args: () => {
    const workletB = (a: { current: number }) => {
      "worklet";
      return a.current;
    };
    const workletA = Worklets.createRunInContextFn(function () {
      "worklet";
      return workletB({ current: 100 });
    });
    return ExpectValue(workletA(), 100);
  },

  fail_when_calling_a_regular_function_from_a_worklet: () => {
    const func = (a: number) => a;
    const worklet = Worklets.createRunInContextFn(function () {
      "worklet";
      return func(100);
    });
    return ExpectException(worklet);
  },
  call_worklet_with_this: () => {
    const obj = {
      a: 100,
      b: 100,
      f: function () {
        "worklet";
        return this.a + this.b;
      },
    };
    const sharedValue = Worklets.createSharedValue(obj);
    const worklet = Worklets.createRunInContextFn(function () {
      "worklet";
      return sharedValue.value.f();
    });
    return ExpectValue(worklet(), 200);
  },
  call_createRunInJsFn_inside_worklet: () => {
    const fn = function (b: number) {
      "worklet";
      return b * 2;
    };
    const f = function (a: number) {
      "worklet";
      const wjs = Worklets.createRunInJsFn(fn);
      return wjs(a);
    };
    const worklet_in_context = Worklets.createRunInContextFn(f);
    return ExpectValue(worklet_in_context(100), 200);
  },
};
