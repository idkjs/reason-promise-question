[@bs.val] [@bs.scope "localStorage"]
external setItem: (string, string) => unit = "setItem";
let setUserToken: string => Js.Promise.t(unit) =
  token => Js.Promise.resolve(setItem("auth", token));

[@bs.val] [@bs.scope "localStorage"]
external getItem: string => Js.Nullable.t(string) = "getItem";
let getUserToken: unit => Js.Promise.t(Js.Nullable.t(string)) =
  () => Js.Promise.resolve(getItem("auth"));

let setTempUserToken: string => Js.Promise.t(unit) =
  _ => Js.Promise.resolve();

let getTempUserToken: unit => Js.Promise.t(Js.Nullable.t(string)) =
  () => Js.Promise.resolve(Js.Nullable.undefined);
