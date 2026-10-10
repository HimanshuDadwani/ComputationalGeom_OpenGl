# OpenGL Geometry Codebase Guide

This guide describes the C++ headers and source files in `CGAlgo`, `OpenGl`, and `OpenGL-Examples`. It is intended for someone new to the repository who wants to understand the existing classes, connect a geometry algorithm to the interactive application, and display results correctly.

## 1. Project Map

| Folder | Responsibility |
| --- | --- |
| `OpenGl` | The reusable application shell: window, frame loop, layers, events, input polling, camera, shaders, debug output, and ImGui/ImPlot integration. |
| `CGAlgo` | Geometry data types, geometric algorithms, numerical solver, and immediate/debug drawing helpers. |
| `OpenGL-Examples` | The client application and example layers that connect input, algorithms, controls, and rendering. |

The common execution flow is:

1. `ExampleApp.cpp` constructs an `Application` and pushes an example `Layer`.
2. `Application` creates its platform `Window`, installs its event callback, and pushes the ImGui overlay.
3. Each frame, `Application::Run()` calls layer updates, starts ImGui, calls layer ImGui functions, renders ImGui, and updates the window.
4. A layer binds its shader, supplies camera uniforms, and draws its content during `OnUpdate`.
5. GLFW callbacks become typed `GLCore::Event` objects and are sent through `Application::OnEvent` to layers.

```text
ExampleApp
  -> Application
       -> Window / WindowsWindow (GLFW + OpenGL context)
       -> LayerStack
            -> ExampleLayer or PendulumLayer
            -> ImGuiLayer overlay
       -> frame loop: OnUpdate -> OnImGuiRender -> swap buffers
```

## 2. Geometry and Algorithms (`CGAlgo`)

### 2.1 Geometry primitives

Defined in `GeometricObjects/header/BasicGeomObjects.h` and implemented where needed in `GeometricObjects/src/BasicGeomObjects.cpp`.

| Type | Purpose and use |
| --- | --- |
| `Point` | A position with public `x`, `y`, and `z`. Supports equality, point subtraction (returns a `Vector`), point-plus-vector, and stream output. Use it for input/control points and geometric results. |
| `Vector` | A 3-component direction or displacement with public `_x`, `_y`, `_z`. Supports scaling, dot product (`a * b`), addition, subtraction, normalization, magnitude, and conversion from `Point`. |
| `Line` | A segment with public endpoints `St` and `End`. `GetIntersection(other, outPoint, lineMode)` attempts a 2D segment intersection; `lineMode=true` accepts line parameters outside the segment interval. |
| `Curve` | Abstract curve interface. Derived curves implement `EvaluateAt(t)`, `FirstDerivative(t)`, and `SecondDerivative(t)`. Bezier parameters are expected in `[0, 1]`. |

**Important primitive limitations:** the `Point(x, y, z)` constructor currently assigns `z` to zero rather than the supplied value, so treat this geometry as 2D. `Line::GetIntersection` returns `void`, does not report parallel/coincident status, and a default output point `(0, 0)` is ambiguous with a valid intersection at the origin. `Vector::Normalize` has no zero-length guard. Account for these when writing robust geometry code.

### 2.2 Bezier curves

`BezierCurve` in `GeometricObjects/header/BezierCurve.h` derives from `Curve` and is implemented in `GeometricObjects/src/BezierCurve.cpp`.

- `SetControlPoints(vector<Point>&)` replaces its control-point list.
- `AddControlPoint(Point&)` appends a point.
- `EvaluateAt(t)` evaluates the Bernstein-form Bezier curve.
- `FirstDerivative(t)` and `SecondDerivative(t)` evaluate the analytic derivatives using differences of neighboring control points and Bernstein polynomials.
- `Draw(true)` rebuilds GPU buffers for its control points, control polygon, sampled curve, and derivative guides. `Draw(false)` reuses existing buffers and issues draw calls.
- `Plot()` currently prepares arrays but its external plotting calls are commented out.
- `TestBerizer()` compares analytic derivatives with finite-difference approximations and asserts on excessive error.
- Nested `BernsteinPoly` stores the polynomial degree and computes one Bernstein basis term. `MathUtilities.h` supplies its recursive `factorial` helper.

For an empty curve, the current class does not provide a safe evaluation contract. Its `NumControlPoints` member is not initialized in the header, so initialize control points before calling evaluation, drawing, or derivative tests. The curve owns `VertexArray`, `VertexBuffer`, and `IndexBuffer` members; constructing/drawing it depends on a current OpenGL context.

### 2.3 Closest point on a curve

`ClosestPointOnCurve` in `GeometricOperations/ClosestPointOnCurve.h/.cpp` finds a parameter where the curve tangent is perpendicular to the vector from a query point to the curve. It uses `NewtonSolver` and produces debug points along the iterative path.

- `Setup(Curve*, Point, initialGuess)` stores the curve and query point, creates the scalar function and derivative, and configures the solver.
- An `initialGuess` of zero triggers a coarse search in `FindInitialGuess`. The current coarse search samples `t = i / 4` for `i = 0..50`, which extends past the nominal Bezier interval; this should be corrected for bounded curves before relying on it.
- `NextStep()` attempts one solver iteration and returns `ClosestPointResult { t, pt, distance }`.
- `Run()` resets and runs the solver, then returns the same result structure.
- `DrawDebugPointAt(t)` adds the corresponding curve point to an internal list and uploads it for display.

The class is not derived from `Algorithm` despite the commented inheritance. Solver results use squared distance on some paths and zero on converged paths, so do not assume `distance` is consistently an ordinary distance without checking/fixing the implementation.

### 2.4 Newton solver

`Solvers/NewtonSolver.h` contains:

- `NewtonResult`: the current/final scalar `x`, a convergence flag, and iteration count.
- `newton(f, df, initialGuess, callback, tolerance, maxIterations)`: a templated one-shot Newton method.
- `NewtonSolver`: stores `std::function` callbacks and settings after `Setup`; `Run()` performs a full solve, `RunIteration()` advances one update, and `Reset()` restores the initial state.

Both forms guard against a near-zero derivative and stop after a configurable iteration limit. The default tolerance is `1e-2` and the default limit is five iterations. `NewtonSolver.cpp` is empty; the implementation is inline in the header.

### 2.5 Bezier intersection API (experimental)

`GeometricObjects/header/BezierIntersection.h` defines:

- `BezierIntersectionResult`: averaged point plus `firstParameter` and `secondParameter`.
- `BezierIntersectionOptions`: position/parameter tolerances, maximum subdivision depth, maximum Newton iterations, and Newton seed count.
- `BezierIntersection::SolveSubdivision(first, second, options)`: recursively prunes parameter ranges using sampled bounds, then refines candidates with a two-parameter Newton iteration.
- `BezierIntersection::SolvePolynomial(first, second, options)`: despite its name, samples a grid of parameter seeds and runs Newton refinement; it does not currently perform polynomial elimination.
- `BezierIntersection::TestIntersection()`: checks a sample pair of quadratic Beziers crossing near `(0.5, 0.5)`.

**This API is not currently integrated and appears not to compile as checked:** `BezierIntersection.cpp` calls `BezierCurve::GetNumControlPoints()`, but that accessor is missing from `BezierCurve.h`. Add/fix that contract and build the target before using the solver. The sampled bounds may not conservatively enclose curve extrema, and tangential/overlapping curves need dedicated tests. No application call site runs `TestIntersection()` or draws its results.

### 2.6 Convex hull

`ConvexHull` in `header/ConvexHull.h` and `src/ConvexHull.cpp`:

- `SetInputPoints(vector<Point>&)` copies an input set.
- `ComputeConvexHull()` sorts by x using `MergeSort`, then builds upper and lower chains using turn tests. It returns its stored hull points and allocates OpenGL buffers.
- `Draw()` draws input points and hull edges.
- `MergeSort` is a free function declared in the same header.
- `TestConvexHull()` is a sample routine in the `.cpp` file.

The implementation expects at least two points and is not robust for empty/singleton input. Its geometry and GPU resources are combined, so use it with a current context. It appends to internal hull storage and should be reset/cleared before repeated computations.

### 2.7 Drawing and debug-rendering helpers

`header/GraphicsUtils.h` and `src/GraphicsUtils.cpp` provide the following:

| Type/function | Responsibility |
| --- | --- |
| `GLCall`, `GLClearError`, `GLCheckError`, `ASSERT` | Clear/check OpenGL error state around a call and break in the debugger on failure. |
| `VertexBuffer` | Owns an array-buffer object; supports bind, unbind, and `SetData`. |
| `IndexBuffer` | Owns an element/index-buffer object; supports bind/unbind, `SetData`, and index count access. |
| `VertexBufferAttribute`, `VertexBufferLayout` | Describe vertex attribute types/counts/stride. `Push<float>(3)` is the common position layout. |
| `VertexArray` | Owns a VAO and captures the buffer/attribute layout through `AddBuffer`. |
| `DrawPoint`, `DrawPoints`, `DrawLine`, `DrawLines`, `DrawLinesWithVertexArray`, `DrawCircle` | Convert `Point`/`Line` data to OpenGL vertices and issue primitive draw calls. Prefer the overloads accepting `VertexArray`/buffer wrappers for retained data. |
| `Renderer` | Holds one point or line renderer, GPU buffers, visibility and update state. A renderer's type is selected by its first point/line render call. |
| `DebugRenderer` | Singleton named collection of `Renderer`s. Add, update, show, hide, test presence, remove, and draw all visible named objects. |

`DebugRenderer::AddObjects(name, points, count)` and its line overload upload data immediately. The collection does not retain the caller's array pointer. Call `UpdateObject` with the same name and same primitive kind when the data changes; call `DrawAllObjects()` every frame to issue draws. Use a stable name and ensure string arguments are mutable `std::string` objects because several APIs take non-const references.

The `Renderer` API is stateful: a name cannot switch from points to lines after its initial type is chosen. The low-level drawing helpers and wrappers require an active OpenGL context and a compatible shader/VAO state.

### 2.8 Other CGAlgo files

- `GeometricOperations/Algorithm.h`: abstract `Start`, `NextStep`, and `Run` interface plus `SetupDone` flag setter. The flag is not initialized, and the current closest-point class does not implement this interface.
- `GeometricOperations/Agorithm.cpp`: empty source file.
- `GeometricObjects/header/MathUtilities.h`: recursive integer `factorial` used by Bernstein terms.
- `src/LineIntersection.cpp` and `header/LineIntersection.h`: the sweep-line classes and test code are commented out; there is no usable line-sweep API here. The active segment intersection method is `Line::GetIntersection`.
- `src/BBST.cpp`: defines a template `BBST<T>` with insert and traversal functions, but the class is in a `.cpp` rather than a reusable header. `main_1()` is a sample entry point with a nonstandard name. It is not connected to the OpenGL application.

## 3. OpenGL Framework (`OpenGl`)

### 3.1 Application and layers

| Class | File | Purpose |
| --- | --- | --- |
| `Application` | `GLCore/Core/Application.h/.cpp` | Singleton-like application owner. Constructs the window, installs event routing, pushes ImGui overlay, dispatches events from topmost layer down, and runs the frame loop. Derive a client application and call `Run()`. |
| `WindowProps` | `GLCore/Core/Window.h` | Title, width, and height passed to the window factory. |
| `Window` | `GLCore/Core/Window.h` | Platform-independent interface for frame update, dimensions, event callback, VSync, and native handle. `Window::Create` selects the implementation. |
| `WindowsWindow` | `Platform/Windows/WindowsWindow.h/.cpp` | GLFW implementation; initializes GLFW/GLAD, makes the context current, sets viewport and event callbacks, polls events, swaps buffers, and configures VSync. |
| `Layer` | `GLCore/Core/Layer.h/.cpp` | Feature/module lifecycle interface. Override `OnAttach`, `OnDetach`, `OnUpdate`, `OnImGuiRender`, and `OnEvent`. |
| `LayerStack` | `GLCore/Core/LayerStack.h/.cpp` | Owns pushed layer pointers, inserts regular layers before overlays, calls `OnAttach` on push and `OnDetach` on pop; destructor deletes layers. |
| `Timestep` | `GLCore/Core/Timestep.h` | Frame delta wrapper convertible to seconds; `GetMilliseconds()` converts to milliseconds. |

`Application::Run()` updates layers before beginning/rendering the ImGui frame, then calls the window's `OnUpdate()` to poll events and swap buffers. `LayerStack::PushLayer`/`PushOverlay` take ownership of the passed pointer; pass heap-allocated layers as shown in the example or adapt ownership consistently.

### 3.2 Events and input

- `Event` is the base event interface with type, category flags, name, string description, and mutable `Handled` status.
- `EventDispatcher::Dispatch<T>(handler)` invokes the handler only if the dynamic event type matches, and assigns the handler's boolean result to `Handled`.
- `WindowResizeEvent`, `WindowCloseEvent`, `AppTickEvent`, `AppUpdateEvent`, and `AppRenderEvent` are in `Events/ApplicationEvent.h`.
- `KeyEvent` is the keyboard base. `KeyPressedEvent` includes repeat count, `KeyReleasedEvent` represents release, and `KeyTypedEvent` represents character input. Defined in `Events/KeyEvent.h`.
- `MouseMovedEvent`, `MouseScrolledEvent`, `MouseButtonEvent`, `MouseButtonPressedEvent`, and `MouseButtonReleasedEvent` are in `Events/MouseEvent.h`. Button events carry cursor coordinates on press in the current Windows callback implementation.
- `Input` is a static polling façade for key/button state and cursor coordinates. `WindowsInput` implements it via GLFW and `Application::Get().GetWindow()`.
- `KeyCodes.h` and `MouseButtonCodes.h` map framework constants to GLFW values.

Use event handlers for discrete actions and polling in `OnUpdate` for continuous movement. Route an event to a helper such as a camera controller, then dispatch feature-specific event types. Return `true` only when the feature has consumed the event so it stops propagating to lower layers.

### 3.3 Camera and coordinate conversion

| Class | Purpose |
| --- | --- |
| `OrthographicCamera` (`GLCore/Util/OrthographicCamera.h/.cpp`) | Holds projection, view, view-projection, and inverse view-projection matrices. Set projection bounds and camera position/rotation. |
| `OrthographicCameraController` (`GLCore/Util/OrthographicCameraController.h/.cpp`) | Updates camera from WASD translation and optional Q/E rotation; handles scroll zoom and resize events. `GetCamera()` exposes the camera and `SetZoomLevel` changes zoom. |
| `CoordinateConverter` (same controller files) | Converts screen pixels to NDC and world coordinates, and world coordinates back to screen. Must be constructed with the camera and updated viewport dimensions. It handles resize events when passed those events. |

`ScreenToWorld` is the correct path for mapping mouse positions to geometry coordinates: screen origin is at the top-left, NDC is generated with y inverted, and the camera's inverse view-projection matrix maps into world coordinates. Make sure the converter and camera have the current viewport size after resize.

### 3.4 Shader, debug, logging, and UI

- `GLCore::Utils::Shader` loads vertex/fragment GLSL text files, compiles and links a program, exposes its OpenGL renderer ID, and deletes the program on destruction. Construct after the OpenGL context exists. Call `glUseProgram(shader->GetRendererID())` before setting uniforms/drawing.
- `GLCore::Utils::OpenGLDebug` provides `EnableGLDebugging`, `SetGLDebugLogLevel`, and the debug message callback. High-severity messages assert at the default `HighAssert` level.
- `GLCore::Log` owns the spdlog logger; `LOG_TRACE`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, and `LOG_CRITICAL` are the client macros.
- `GLCore::ImGuiLayer` creates/destroys ImGui and ImPlot contexts, begins/ends each UI frame, and forwards selected mouse events. Check `ImGui::GetIO().WantCaptureMouse` before treating a mouse action as a scene click.
- `ImGuiBuild.cpp` compiles the GLFW/OpenGL3 ImGui backends. `DrawShapes.cpp` and `glpch.cpp` contain no additional class API in the inspected source.
- `GLCore.h` is the client umbrella include for GLM, GLAD, ImGui, ImPlot, and `Application`. `GLCoreUtils.h` includes shader, camera/controller/converter, and OpenGL-debug APIs. `glpch.h` is the framework precompiled-header include.

## 4. Example Application (`OpenGL-Examples`)

### 4.1 `ExampleApp.cpp` and `ExampleLayer`

`ExampleApp.cpp` defines the executable's `Example` subclass of `Application`, pushes `ExampleLayer`, and calls `Run()` in `main`. `PendulumLayer` is present but not enabled in that constructor.

`ExampleLayer` in `src/ExampleLayer.h/.cpp` demonstrates the main client patterns:

- `OnAttach`: enable GL debugging and depth/blend state; load the test shader files.
- `OnUpdate`: clear the frame, bind the shader, update the camera, upload `u_ViewProjection` and `u_Color`, draw axes/Bezier content, then call `DebugRenderer::DrawAllObjects()`.
- `OnEvent`: pass events to `OrthographicCameraController` and `CoordinateConverter`, then dispatch mouse button events.
- `OnImGuiRender`: build controls and plots, gate scene interactions on ImGui capture state, read cursor input, convert it to world coordinates, add Bezier control points, and start closest-point operations.
- `GenerateAxes`: initializes a VAO/VBO/IBO once and issues indexed line drawing on later frames.
- `GenerateConvexHull`: prepares sample points, computes the hull once, and draws it.
- `GenerateBZ`: initializes a sample Bezier curve, runs derivative checks, rebuilds its draw buffers, then reuses them. Its current sample coordinates are much larger than the camera's initial view bounds, which can make the curve appear off-screen until zoom/pan or coordinate choices are adjusted.
- `GenerateLineIntersection`: currently does nothing; its test call is commented out.

`ExampleLayer` members include the shader, camera controller, coordinate converter, axes GL IDs, colors, one closest-point operation, a convex hull, a Bezier curve, and a point VAO/VBO/EBO for the closest-point display.

### 4.2 Pendulum examples

- `Pendulum` is defined inside `PendulumLayer.h`. It stores a simple pendulum's state and parameters, advances an Euler integration in `update(time)`, and directly draws the rod and bob. `SetTorque` and `SetDampingCoeff` change simulation parameters.
- `InvertedPendulum` is declared in `InvertedPendulum.h` and implemented in `Invertedpendulum.cpp`. It stores a four-component state (base position, angle, base velocity, angular rate), physical parameters, and `Update(time, force)` integrates the equations using Euler steps. `GetState` returns the current state; `drawLine`/`drawCircle` render the system.
- `PendulumLayer` creates a simple triangle buffer in `OnAttach`, updates both pendulum models and draws them in `OnUpdate`, forwards events to the camera, and exposes torque/damping sliders in ImGui.
- `matplotlibcpp.h` is a vendored plotting wrapper, not part of the framework's geometry/rendering API. Example code using it is commented out; the active plots use ImPlot.
- `ConvexHull_old.h` is entirely commented legacy code; use the active CGAlgo `ConvexHull` instead.

The pendulum classes issue OpenGL calls directly and create buffers in their constructors. Construct them only after a valid context exists. The simple pendulum sample also keeps unused VAO member IDs; do not assume its resource lifecycle is a general rendering pattern.

## 5. How To Add A Feature

### 5.1 Keep algorithm, presentation, and interaction separate

For a new geometry operation, a clean division is:

1. **Geometry/data:** reuse `Point`, `Vector`, `Line`, and `Curve` from `BasicGeomObjects.h` where their semantics fit.
2. **Algorithm:** add a result type and options type plus an operation class in `CGAlgo/GeometricOperations` or the relevant geometric-object folder. Prefer calculation APIs that return standard containers and do not draw as a side effect.
3. **Verification:** add deterministic test inputs with known expected results. Test empty/degenerate inputs, endpoints, tolerance behavior, repeated results, and near-tangent cases as relevant.
4. **Example integration:** store the operation and its input geometry in an `OpenGL-Examples` layer. Trigger computation from an ImGui action or a discrete event.
5. **Display:** translate result points/segments to `DebugRenderer` objects or a layer-owned VAO/VBO/IBO; update the GPU data only when the results change.

This keeps the numerical operation reusable and makes visual debugging a client responsibility rather than a requirement for every algorithm test.

### 5.2 Curve intersection: concrete steps

There is already a `BezierIntersection` API scaffold, but first resolve its compile/API mismatch: either add a safe `GetNumControlPoints() const` accessor to `BezierCurve`, or change the solver's empty-input check to use a public curve API. Initialize the control-point count to zero and make evaluation on an empty curve explicit before relying on the solver. Then:

1. Build two curves from independent `vector<Point>` control-point lists.
2. Configure `BezierIntersectionOptions` explicitly for the test scale and expected precision.
3. Call both candidate solvers and compare their result sets against known fixtures. A useful first fixture is the crossing pair in `BezierIntersection::TestIntersection()`; extend it with no-crossing, endpoint-crossing, multiple-crossing, tangent, coincident, and near-parallel cases.
4. Validate every result by re-evaluating both curves at the returned parameters and checking that the point gap is within `positionTolerance`. Check parameter bounds and deduplicate by parameter and/or spatial tolerance.
5. Once the algorithm passes tests, add an ImGui button or keyboard action in `ExampleLayer`. Compute only when inputs change or the user requests it, not unconditionally each frame.
6. Display returned `result.point` values using stable names with `DebugRenderer::AddObjects` initially and `UpdateObject` for subsequent result changes. Call `DrawAllObjects()` every update frame. Optionally draw lines or tangent vectors for debugging.
7. If users can place control points with the mouse, call `Input::GetMousePosition()`, check `!ImGui::GetIO().WantCaptureMouse`, convert with `m_CoordController.ScreenToWorld(...)`, and then update the correct curve's control-point list.

Example call site once the API compiles:

```cpp
BezierCurve firstCurve;
BezierCurve secondCurve;
firstCurve.SetControlPoints(firstControlPoints);
secondCurve.SetControlPoints(secondControlPoints);

BezierIntersectionOptions options;
options.positionTolerance = 1e-5;
const auto intersections =
    BezierIntersection::SolveSubdivision(firstCurve, secondCurve, options);

for (const BezierIntersectionResult& result : intersections)
{
    // Use result.point, result.firstParameter, and result.secondParameter.
}
```

Because `BezierCurve` currently owns OpenGL buffer wrappers, constructing it in a test may require an initialized GL context. For headless unit tests, split CPU-only curve/algorithm data from GPU rendering resources, or ensure the test fixture creates a context before constructing those objects.

## 6. Rendering Correctly

For a new drawable object, follow the order demonstrated in `ExampleLayer`:

1. **Have a context:** create GL resources only after the window has initialized GLFW and GLAD. Layer `OnAttach` runs after `Application` creates the window and GL context.
2. **Prepare shader state:** load the GLSL shader in `OnAttach`; each frame call `glUseProgram`. Use the shader's expected attribute locations and uniforms. The example shader expects `u_ViewProjection` and `u_Color`.
3. **Use world-space coordinates:** algorithm results and curve control points should be in the camera's world coordinate system. Do not mix pixel coordinates with geometry coordinates.
4. **Keep camera and viewport synchronized:** update `OrthographicCameraController` with each timestep and forward resize events. Update the coordinate converter with resize events as well.
5. **Create valid vertex layout:** positions in current helpers are uploaded as three floats `(x,y,z)` and use vertex attribute 0. Configure VAO/VBO/IBO consistently with the draw call (`GL_LINES`, `GL_POINTS`, etc.).
6. **Upload at the right time:** on new/changed geometry, call drawing helpers with `RecomputeBuffer=true`; in later frames reuse the buffers. Passing no geometry with `RecomputeBuffer=false` should draw existing uploaded data.
7. **Draw each frame:** clear the color/depth buffers, bind program and camera uniforms, issue the draw, then render debug objects. Never rely on a one-time draw call persisting to the next backbuffer frame.
8. **Fit the view:** the initial orthographic camera spans approximately `[-100, 100]` vertically with horizontal range scaled by aspect ratio. Coordinates outside that frustum will be clipped. Use camera zoom/pan or choose a matching coordinate scale.
9. **Clean up with a live context:** delete GL resources in `OnDetach` or resource destructors that run before the context/window is destroyed. Avoid duplicate deletion and ensure owned GL IDs are initialized.

The app's window-size callback sets `glViewport` using framebuffer dimensions. For high-DPI windows, note the coordinate converter currently uses logical window event dimensions, so verify mouse-to-world mapping on scaled displays.

## 7. How To Read And Handle User Input

### Continuous keyboard input

Poll through `GLCore::Input` in `OnUpdate`:

```cpp
#include "GLCore/Core/KeyCodes.h"

if (GLCore::Input::IsKeyPressed(HZ_KEY_SPACE))
{
    // Continuous action while the key is held.
}
```

Use `Timestep` when changing positions or simulation state so behavior is independent of frame rate. The camera controller demonstrates this approach for WASD and Q/E.

### Discrete key/mouse events

Override `OnEvent` and dispatch a typed event:

```cpp
GLCore::EventDispatcher dispatcher(event);
dispatcher.Dispatch<GLCore::KeyPressedEvent>([&](GLCore::KeyPressedEvent& keyEvent)
{
    // Inspect keyEvent.GetKeyCode() and GetRepeatCount().
    return false;
});
```

Events flow from `Application` to the layer stack in reverse order. A handler's `true` result marks the event handled and stops propagation. Forward events to the camera and coordinate converter before custom dispatch if the layer uses them.

### Mouse placement in the scene

The example uses this pattern for adding Bezier points:

```cpp
if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseClicked(0))
{
    const auto mouse = GLCore::Input::GetMousePosition();
    const glm::vec2 world = m_CoordController.ScreenToWorld({ mouse.first, mouse.second });
    m_BZ.AddControlPoint(Point(world.x, world.y));
    m_BZ.Draw(true);
}
```

Use the event's mouse coordinates for event-driven handling, or the polling API when checking current cursor state. In either case, screen pixels must pass through `CoordinateConverter` before they are treated as geometry positions. Gate scene clicks while ImGui is capturing mouse input.

## 8. Practical Caveats And Readiness

- `BezierIntersection` has a missing `BezierCurve::GetNumControlPoints()` API dependency and needs a successful build before it can be treated as usable.
- The line sweep implementation is commented out; `ExampleLayer::GenerateLineIntersection()` currently does not compute anything.
- `ConvexHull` assumes enough points, combines computation with GL allocation, and does not clearly reset internal results for repeated calls.
- `BezierCurve` and several other CGAlgo types have GL members, so “algorithm only” tests can accidentally require a graphics context.
- The closest-point coarse guess can sample outside `[0, 1]`; Newton iteration can leave the curve's domain, and reported distance semantics vary by result path.
- `DebugRenderer` requires a stable primitive kind per object name and valid GL context. Initial upload uses `AddObjects`; later changes use `UpdateObject`.
- Some public methods use non-const reference parameters for inputs that do not need mutation. Match those signatures or improve them as part of a separate API cleanup.
- This guide intentionally covers `.h` and `.cpp` source only. Build-system files, assets, and dependency setup were not inspected for this documentation pass.

## 9. Source File Index

The primary inspected source groups are:

- `CGAlgo/GeometricObjects/header` and `CGAlgo/GeometricObjects/src`: primitive types, Bezier curve, Bezier intersection, and math utilities.
- `CGAlgo/GeometricOperations`: algorithm interface and closest-point operation.
- `CGAlgo/Solvers`: Newton solver.
- `CGAlgo/header` and `CGAlgo/src`: convex hull, graphics utilities, commented line-intersection experiment, and BBST experiment.
- `OpenGl/src/GLCore/Core`: application, window interface, layers, input interface, logging, timestep, and key/button codes.
- `OpenGl/src/GLCore/Events`: event base, dispatcher, and event types.
- `OpenGl/src/GLCore/ImGui`: UI overlay and backend compilation unit.
- `OpenGl/src/GLCore/Util`: shader, camera/controller/converter, OpenGL debug, and draw-shape source.
- `OpenGl/src/Platform/Windows`: GLFW window and input implementations.
- `OpenGL-Examples/src`: executable, interactive geometry example, pendulum layers/models, plotting wrapper, and legacy commented hull source.