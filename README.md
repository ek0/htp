# Hook The Planet

Dynamic analysis framework for windows executables.

## Folder architecture

- injector: Holds the logic to inject the agent within a target windows process.
- agent: Code that will be embedded in the DLL to be injected in the target windows process.
- external: Third party libraries.
- shared: code shared between multiple components.
- include: contains the public header files.

## Using the SDK

- Build the SDK
- Create your DLL project, statically link agent.lib
- Inject it using the injector (or any way you want)

## TODO:
- Define API (preferably start with AgentInit() returning an opaque pointer).
- Refactor refactor refactor...
- Write tests.
- Write I/O component
- Replace all printf with the I/O module