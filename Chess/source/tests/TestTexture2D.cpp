#include "TestTexture2D.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "imgui/imgui.h"
#include "Texture.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "CreateQuadDynamic.h"
#include "ChessThings/Board.h"
#include "ChessThings/RenderChesspcs.h"

namespace test {
	TestTexture2D::TestTexture2D()
		:m_Proj(glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f)),
		m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))),
		m_TranslationA(353,363,0), m_TranslationB(400,200,0)
		
	{

		glm::vec3 translationA(200, 200, 0);


		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		m_VAO = std::make_unique<VertexArray>();
		
		m_VertexBuffer = std::make_unique<VertexBuffer>(nullptr, sizeof(VertexStructure)*2000);
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(2);
		layout.Push<float>(1);
		m_VAO -> AddBuffer(*m_VertexBuffer, layout);
		m_IndexBuffer = std::make_unique<IndexBuffer>(130);

		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
		m_Shader->Bind();

		Texture texture;
		TEX_ChessBoard = texture.CreateTexture("res/textures/Chessboard.png");
		TEX_P = texture.CreateTexture("res/textures/wp.png");
		TEX_B = texture.CreateTexture("res/textures/wb.png");
		TEX_N = texture.CreateTexture("res/textures/wn.png");
		TEX_R = texture.CreateTexture("res/textures/wr.png");
		TEX_Q = texture.CreateTexture("res/textures/wq.png");
		TEX_K = texture.CreateTexture("res/textures/wk.png");
		TEX_p = texture.CreateTexture("res/textures/bp.png");
		TEX_b = texture.CreateTexture("res/textures/bb.png");
		TEX_n = texture.CreateTexture("res/textures/bn.png");
		TEX_r = texture.CreateTexture("res/textures/br.png");
		TEX_q = texture.CreateTexture("res/textures/bq.png");
		TEX_k = texture.CreateTexture("res/textures/bk.png");
		TEX_Empty = texture.CreateTexture("res/textures/Empty.png");
		TEX_Red = texture.CreateTexture("res/textures/Red.png");
		//texture sampling part
		int samplers[15] = { 0, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 10, 11, 12 ,13, 14 };
		m_Shader->SetUniform1iv("u_Textures", 15, *samplers);

		

	}

	TestTexture2D::~TestTexture2D()
	{

	}

	void TestTexture2D::OnUpdate(float deltaTime)
	{

	}

	void TestTexture2D::OnRender(GLFWwindow* window)
	{
		Board board;
		RenderChessPieces renderChessPieces;
		renderChessPieces.GetMouseInput(window);
		renderChessPieces.SetStaticBoardSquare(board.GetPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1"));
		auto position = renderChessPieces.MemcopyObjects(renderChessPieces.CreateObjects());
		
		//just dont look at this
		GLCall(glBindTextureUnit(0, TEX_ChessBoard));
		GLCall(glBindTextureUnit(7, TEX_P));
		GLCall(glBindTextureUnit(8, TEX_B));
		GLCall(glBindTextureUnit(9, TEX_N));
		GLCall(glBindTextureUnit(10, TEX_R));
		GLCall(glBindTextureUnit(11, TEX_Q));
		GLCall(glBindTextureUnit(12, TEX_K));
		GLCall(glBindTextureUnit(1, TEX_p));
		GLCall(glBindTextureUnit(2, TEX_b));
		GLCall(glBindTextureUnit(3, TEX_n));
		GLCall(glBindTextureUnit(4, TEX_r));
		GLCall(glBindTextureUnit(5, TEX_q));
		GLCall(glBindTextureUnit(6, TEX_k));
		GLCall(glBindTextureUnit(13, TEX_Empty));
		GLCall(glBindTextureUnit(14, TEX_Red));

		m_VertexBuffer->SetDynamicVB(&position, sizeof(position));

		Renderer renderer;
		
		//TODO: Actually, i have no idea how to fix the artifacts

		renderChessPieces.WasLeftButtonPressed();

		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationA);
			glm::mat4 mvp = m_Proj * m_View * model;
			m_Shader->Bind();
			m_Shader->SetUniformMat4f("u_MVP", mvp);
			renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader);
		}
		
	}

	void TestTexture2D::OnImGuiRender()
	{
		ImGui::SliderFloat3("Translation A", &m_TranslationA.x, 0.0f, 1280.0f);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	}

}