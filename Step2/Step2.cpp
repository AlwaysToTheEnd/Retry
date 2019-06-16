#include "Step2.h"
#include "SceneSimulationEventCallBack.h"
#include <random>

std::mt19937_64 g_random(710);

Step2::Step2(HINSTANCE hInstance)
	: D3DApp(hInstance)
	, m_driveJoint(nullptr)
{

}

Step2::~Step2()
{
	m_planeMaterial = nullptr;
	m_scene = nullptr;
	m_dispatcher = nullptr;
	m_cooking = nullptr;
	m_cudaManager = nullptr;
	m_physics = nullptr;

	PxPvdTransport* transport = m_PVD->getTransport();
	m_PVD = nullptr;
	transport->release();

	m_foundation = nullptr;
	m_foundation = nullptr;
}

bool Step2::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	InitPhsyX();
	InitObjects();

	return true;
}

void Step2::Update()
{
	MainPassUpdate();
	PhysXBaseUpdate();
}

void Step2::InitPhsyX()
{
	// ���� ���ʰ� �Ǵ� Foundation ����, �̱��� Ŭ�����ӿ� ��������.
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
		m_allocator, m_errorCallback);

	// Physics Visual Debugger�� �����ϱ� ��Ʈ ������ ���� �۾�. 
	// ������� �ʴ´ٸ� ���� �ʾƵ� �����ϴ�.

	m_PVD = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// ��� Physics ���ҽ� ������ ���� ���� Ŭ���� ����, �̱��� Ŭ�����ӿ� ��������.
	// DirectX Device�� ���� �������� �����ϸ� ����.
	// PxTolerancesScale ��� ����ó���� �־ ������ �Ǵ� ���� ����.
	// length(1) 
	// mass(1000) 
	// speed(10) 

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
		PxTolerancesScale(),
		true, m_PVD.Get());
	PxRegisterHeightFields(*m_physics);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	// CPU�۾��� ������� ���� ���Ѵ�. ���� ����
	// �������� ���� ���Ѵ�.
	m_dispatcher = PxDefaultCpuDispatcherCreate(info.dwNumberOfProcessors);

	/*MyGPULoadHook loadHook;
	PxSetPhysXGpuLoadHook(&loadHook);

	PxCudaContextManagerDesc cudaDesc;
	cudaDesc.graphicsDevice = m_device.Get();
	cudaDesc.interopMode = PxCudaInteropMode::D3D10_INTEROP;
	m_cudaManager = PxCreateCudaContextManager(*m_foundation, cudaDesc);
	int isPhysXGPU = PxGetSuggestedCudaDeviceOrdinal(m_errorCallback);*/

	// ������Ʈ���� �ùķ��̼� �� ������ �����Ѵ�.
	// �� �ȿ� �ùķ��̼� �� ���� �� �� ���� ���� �������� �ùķ��̼� �Ѵ�.
	// PxSceneDesc�� ���� �Ӽ� ������ ���� ����ü�̴�.
	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher.Get();
	//sceneDesc.gpuDispatcher = m_cudaManager->getGpuDispatcher();
	sceneDesc.broadPhaseType = PxBroadPhaseType::eABP;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	//sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;

	// �������� �ùķ��̼�,���� �̺�Ʈ���� �ݹ� �������̽��� �����Ѵ�

	// �ݹ��Ģ
	// �ݹ��� ���ν����� �Ǵ� �ùķ��̼� �����忡�� ���ÿ� ����� �� �����Ƿ�
	// SDK�� ���¸� �����ϸ� �ȵǸ� Ư�� ��ü�� �����ϰų� �ı��ؼ��� �ȵȴ�
	// ���� ������ �ʿ��� ��� ���� ������ ���ۿ� ���� �� �ùķ��̼� �ܰ� ���� ������ ��
	static SceneSimulationEventCallBack custumSimulationEvent;
	sceneDesc.simulationEventCallback = &custumSimulationEvent;

	m_scene = m_physics->createScene(sceneDesc);

	//pvd Ŭ���̾�Ʈ ����
	PxPvdSceneClient* pvdClient = m_scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		m_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
	}

	// �뷮�� �����͸� �����ϰ� ��ȯ, ����ȭ �ϴ� ��ƿ��Ƽ ����
	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation,
		PxCookingParams(PxTolerancesScale()));
}

void Step2::InitObjects()
{

}

void Step2::PhysXBaseUpdate()
{
	m_scene->simulate(1.0f / 60.0f);
	m_scene->fetchResults(true);
}

void Step2::MainPassUpdate()
{
	m_camera.Update();
	m_GDevice->Update();
}

PxFilterFlags Step2::ScissorFilter(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags & pairFlags, const void * constantBlock, PxU32 constantBlockSize)
{
	return PxFilterFlags();
}


LRESULT Step2::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_camera.WndProc(hwnd, msg, wParam, lParam);

	return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Step2 theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (std::exception e)
	{
		std::string stringMessage(e.what());
		std::wstring exceptionMessage(stringMessage.begin(), stringMessage.end());
		MessageBox(nullptr, exceptionMessage.c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}