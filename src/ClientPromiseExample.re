let graphqlEndpoint = "localhost:4000";

let headers = {"high": "five"};

let httpLink =
  ApolloClient.Link.HttpLink.make(
    ~uri=_ => "http://" ++ graphqlEndpoint,
    ~headers=Obj.magic(headers),
    (),
  );
let authLink =
  ApolloClient.Link.ContextLink.makeAsync((~operation as _, ~prevContext as ctx) => {
    Tokens.getUserToken()
    ->Js.Promise.then_(
        token => {
          switch (token->Js.Nullable.toOption) {
          | None =>
            Tokens.getTempUserToken()
            ->Js.Promise.then_(
                token => Js.Promise.resolve(Js.Nullable.toOption(token)),
                _,
              )
          | Some(token) => Js.Promise.resolve(Some(token))
          }
        },
        _,
      )
    ->Js.Promise.then_(
        fun
        | None => Js.Promise.resolve(Js.Json.null)
        | Some(token) => {
            Js.Promise.resolve(
              [%raw
                {| (context, token) => ({
                headers: {
                  ...ctx.headers,
                  authorization: `Bearer ${token}`
                }
              }) |}
              ](
                ctx,
                token,
              ),
            );
          },
        _,
      )
  });
let wsLink =
  ApolloClient.Link.WebSocketLink.(
    make(
      ~uri="ws://" ++ graphqlEndpoint,
      ~options=
        ClientOptions.make(
          ~connectionParams=
            ConnectionParams(Obj.magic({"headers": headers})),
          ~reconnect=true,
          (),
        ),
      (),
    )
  );

let terminatingLink =
  ApolloClient.Link.split(
    ~test=
      ({query}) => {
        let definition = ApolloClient.Utilities.getOperationDefinition(query);
        switch (definition) {
        | Some({kind, operation}) =>
          kind === "OperationDefinition" && operation === "subscription"
        | None => false
        };
      },
    ~whenTrue=wsLink,
    ~whenFalse=httpLink,
  );

let instance =
  ApolloClient.(
    make(
      ~cache=Cache.InMemoryCache.make(),
      ~connectToDevTools=true,
      ~defaultOptions=
        DefaultOptions.make(
          ~mutate=
            DefaultMutateOptions.make(
              ~awaitRefetchQueries=true,
              ~errorPolicy=All,
              (),
            ),
          ~query=
            DefaultQueryOptions.make(
              ~fetchPolicy=NetworkOnly,
              ~errorPolicy=All,
              (),
            ),
          ~watchQuery=
            DefaultWatchQueryOptions.make(
              ~fetchPolicy=NetworkOnly,
              ~errorPolicy=All,
              (),
            ),
          (),
        ),
      ~link=terminatingLink,
      (),
    )
  );
