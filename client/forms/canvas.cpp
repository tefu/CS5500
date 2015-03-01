// Example application modified from the wxWidgets demo here:
// http://fossies.org/dox/wxWidgets-3.0.2/cube_8cpp_source.html
#include "logger.h"
#include "canvas.h"

// control ids
enum
{
  GameTimer = wxID_HIGHEST + 1
};

wxBEGIN_EVENT_TABLE(GameLoopCanvas, wxGLCanvas)
  EVT_PAINT(GameLoopCanvas::OnPaint) EVT_KEY_DOWN(GameLoopCanvas::OnKeyDown)
  EVT_TIMER(GameTimer, GameLoopCanvas::OnGameTimer)
  EVT_MOTION(GameLoopCanvas::OnMouseUpdate) wxEND_EVENT_TABLE()

  GameLoopCanvas::GameLoopCanvas(wxWindow* parent, wxSize size, int* attribList)
  : wxGLCanvas(parent,
               wxID_ANY,
               attribList,
               wxDefaultPosition,
               size,
               wxFULL_REPAINT_ON_RESIZE)
  , m_spinTimer(this, GameTimer)
{
  GameInit();
}

GameLoopCanvas::~GameLoopCanvas()
{
  delete chunk_manager;
}

void GameLoopCanvas::GameInit()
{
  m_spinTimer.Start(60); // in milliseconds.
  chunk_manager = new ChunkManager();

  // Just an example of how to use the ChunkManager
  for (int i = 0; i < Chunk::CHUNK_SIZE * 3; i++)
  {
    for (int j = 0; j < Chunk::CHUNK_SIZE * 2; j++)
    {
      for (int k = 0; k < Chunk::CHUNK_SIZE * 3; k++)
      {
        BlockType type;
        auto branchOn = rand() % 1000;
        if (branchOn < 990)
        {
          type = BlockType::Inactive;
        }
        else if (branchOn < 991)
        {
          type = BlockType::Ground;
        }
        else if (branchOn < 992)
        {
          type = BlockType::Water;
        }
        else if (branchOn < 993)
        {
          type = BlockType::Sand;
        }
        else if (branchOn < 994)
        {
          type = BlockType::Wood;
        }
        else if (branchOn < 995)
        {
          type = BlockType::Stone;
        }
        else if (branchOn < 996)
        {
          type = BlockType::Grass;
        }
        else if (branchOn < 998)
        {
          type = BlockType::Brick;
        }
        else 
        {
          type = BlockType::Party;
        }

        chunk_manager->set(i, j, k, type);
      }
    }
  }

  position = glm::vec3(3, 3, 3);
  player_angle = glm::vec3(0, -0.5, 0);
  up = glm::vec3(0, 1, 0);
  VectorUpdate(player_angle);
  background_color = 0.5;
  steal_mouse = true;
  mouse_changed = false;
}


// Needed to use wxGetApp(). Usually you get the same result
// by using wxIMPLEMENT_APP(), but that can only be in main.
DECLARE_APP(MyApp);

void GameLoopCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  // This is required even though dc is not used otherwise.
  wxPaintDC dc(this);
  Render();
}

void GameLoopCanvas::Resize()
{
  auto ClientSize = GetClientSize();
  glViewport(0, 0, ClientSize.x, ClientSize.y);
}

void GameLoopCanvas::OnKeyDown(wxKeyEvent& event)
{
  LOG(DEBUG) << "GameLoopCanvas::" << __FUNCTION__;

  switch (event.GetKeyCode())
  {
  case WXK_RIGHT:
  case 'D':
    position += right * player_speed;
    break;

  case WXK_LEFT:
  case 'A':
    position -= right * player_speed;
    break;

  case WXK_UP:
  case 'W':
    position += forward * player_speed;
    break;

  case WXK_DOWN:
  case 'S':
    position -= forward * player_speed;
    break;

  case 'K':
    position += up * player_speed;
    break;

  case 'J':
    position -= up * player_speed;
    break;

  case WXK_ESCAPE:
    // Toggle stealing the mouse.
    steal_mouse = (steal_mouse) ? false : true;
    break;

  default:
    event.Skip();
    return;
  }
}

void GameLoopCanvas::OnGameTimer(wxTimerEvent& WXUNUSED(event))
{
  Update();
  Render();
}

void GameLoopCanvas::Render()
{
  LOG(DEBUG) << "Rendering";
  // With perspective OpenGL graphics, the wxFULL_REPAINT_ON_RESIZE style
  // flag should always be set, because even making the canvas smaller should
  // be followed by a paint event that updates the entire canvas with new
  // viewport settings.
  Resize();

  // Clear screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Background color
  glClearColor(background_color, background_color, background_color, 1.0);

  // The context contains all the graphics utils.
  GraphicsContext& context = wxGetApp().GetContext(this);

  auto program = context.shaderProgram();
  auto MatrixID = glGetUniformLocation(program, "mvp");
  auto Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

  // Camera matrix
  auto View = glm::lookAt(position, position + lookat, up);
  // Just generate our view and projection; the model will be chosen
  // by individual chunks.
  auto VP = Projection * View;

  auto attribute_coord = glGetAttribLocation(program, "coord");
  chunk_manager->render(VP, MatrixID, attribute_coord);

  SwapBuffers();
}

void GameLoopCanvas::Update()
{
  VectorUpdate(player_angle);
  chunk_manager->update();
}

void GameLoopCanvas::VectorUpdate(glm::vec3 angle)
{

  // We want the forward and right vectors to be updated based on
  // where the player is rotated.
  forward.x = sinf(angle.x);
  forward.y = 0;
  forward.z = cosf(angle.x);

  right.x = -cosf(angle.x);
  right.y = 0;
  right.z = sinf(angle.x);

  lookat.x = sinf(angle.x) * cosf(angle.y);
  lookat.y = sinf(angle.y);
  lookat.z = cosf(angle.x) * cosf(angle.y);
}

void GameLoopCanvas::OnMouseUpdate(wxMouseEvent& event)
{
  if (steal_mouse)
  {
    // Ignore every other mouse event so the mouse has a chance to
    // change before being warped to the center.
    if (mouse_changed)
    {
      mouse_changed = false;
      auto x = event.GetPosition().x;
      auto y = event.GetPosition().y;
      auto size = GetSize();

      // Update the angle the player is facing.
      player_angle.x -= (x - (size.x / 2)) * 0.001;
      player_angle.y -= (y - (size.y / 2)) * 0.001;

      // Check if the x angle has gone full circle.
      player_angle.x += (player_angle.x < -M_PI) ? 2 * M_PI : 0;
      player_angle.x -= (player_angle.x > M_PI) ? 2 * M_PI : 0;

      // Stop the y angle from going over 90 degrees away from the
      // horizontal.
      player_angle.y =
        (player_angle.y < -M_PI / 2) ? -M_PI / 2 : player_angle.y;
      player_angle.y = (player_angle.y > M_PI / 2) ? M_PI / 2 : player_angle.y;

      WarpPointer(size.x / 2, size.y / 2);
    }
    else
    {
      mouse_changed = true;
    }
  }
}
