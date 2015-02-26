// Example application modified from the wxWidgets demo here:
// http://fossies.org/dox/wxWidgets-3.0.2/cube_8cpp_source.html
#include "canvas.h"

// control ids
enum
{
  SpinTimer = wxID_HIGHEST + 1
};

// ----------------------------------------------------------------------------
// TestGLCanvas
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas) EVT_PAINT(TestGLCanvas::OnPaint)
  EVT_KEY_DOWN(TestGLCanvas::OnKeyDown)
  EVT_TIMER(SpinTimer, TestGLCanvas::OnSpinTimer) wxEND_EVENT_TABLE()

  TestGLCanvas::TestGLCanvas(wxWindow* parent, int* attribList)
  : wxGLCanvas(parent,
               wxID_ANY,
               attribList,
               wxDefaultPosition,
               wxDefaultSize,
               wxFULL_REPAINT_ON_RESIZE)
  , m_spinTimer(this, SpinTimer)
{
  GameInit();
}

TestGLCanvas::~TestGLCanvas()
{
  delete chunk_manager;
}

void TestGLCanvas::GameInit()
{
  // Init for the game loop.
  m_spinTimer.Start(60); // in milliseconds.
  chunk_manager = new ChunkManager();

  // Fill the first few chunks.
  for (int i = 0; i < Chunk::CHUNK_SIZE * 2; i++)
  {
    for (int j = 0; j < Chunk::CHUNK_SIZE * 2; j++)
    {
      for (int k = 0; k < Chunk::CHUNK_SIZE * 2; k++)
      {
        chunk_manager->set(i, j, k, BlockType::Active);
      }
    }
  }

  position = glm::vec3(3, 3, 3);
  angle = glm::vec3(0, -0.5, 0);
  up = glm::vec3(0, 1, 0);
  VectorUpdate(angle);
  background_color = 0.5;
}

// Needed to use wxGetApp(). Usually you get the same result
// by using wxIMPLEMENT_APP(), but that can only be in main.
wxDECLARE_APP(MyApp);

void TestGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  // This is required even though dc is not used otherwise.
  wxPaintDC dc(this);
  Render();
}

void TestGLCanvas::Resize()
{
  auto ClientSize = GetClientSize();
  glViewport(0, 0, ClientSize.x, ClientSize.y);
}

void TestGLCanvas::OnKeyDown(wxKeyEvent& event)
{
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

  default:
    event.Skip();
    return;
  }
}

void TestGLCanvas::OnSpinTimer(wxTimerEvent& WXUNUSED(event))
{
  Update();
  Render();
}

void TestGLCanvas::Render()
{
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
  TestGLContext& context = wxGetApp().GetContext(this);

  auto program = context.shaderProgram();
  auto MatrixID = glGetUniformLocation(program, "mvp");
  auto Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

  // Camera matrix
  auto View = glm::lookAt(position, position + lookat, up);
  // Just generate our view and projection; the model will be chosen
  // by individual chunks.
  auto VP = Projection * View;

  // 1rst attribute buffer : vertices
  chunk_manager->render(VP, MatrixID);

  SwapBuffers();
}

void TestGLCanvas::Update()
{
  VectorUpdate(angle);
  chunk_manager->update();
}

void TestGLCanvas::VectorUpdate(glm::vec3 angle)
{
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
