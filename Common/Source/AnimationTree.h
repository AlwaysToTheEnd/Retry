#pragma once
#include <string>
#include <vector>
#include <assert.h>

namespace AniTree
{
	enum DataType
	{
		TYPE_BOOL,
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_UINT,
	};

	enum CHANGE_CONDITION_TYPE
	{
		CHANGE_CONDITION_TYPE_ANI_END,
		CHANGE_CONDITION_TYPE_TRIGGER
	};

	enum TRIGGER_TYPE
	{
		CHANGE_CONDITION_TYPE_TRIGGER_GRATER,
		CHANGE_CONDITION_TYPE_TRIGGER_LESS,
		CHANGE_CONDITION_TYPE_TRIGGER_SAME
	};

	class TriggerData
	{
	public:
		TRIGGER_TYPE triggerType;
		DataType dataType;
		union
		{
			bool	data_b;
			float	data_f;
			int		data_i;
			unsigned int data_u;
		};

		void* targetVariable = nullptr;

	private:
		template<typename T>
		bool CheckData(TRIGGER_TYPE triggerTYPE, T a, T b)
		{
			switch (conditionType)
			{
			case AniTree::CHANGE_CONDITION_TYPE_TRIGGER_GRATER:
				return a > b;
				break;
			case AniTree::CHANGE_CONDITION_TYPE_TRIGGER_LESS:
				return a < b;
				break;
			case AniTree::CHANGE_CONDITION_TYPE_TRIGGER_SAME:
				return a == b;
				break;
			default:
				break;
			}

			return false;
		}

	public:
		bool IsTriggerOK()
		{
			switch (dataType)
			{
			case AniTree::TYPE_BOOL:
				return CheckData(triggerType, *reinterpret_cast<bool*>(targetVariable), data_b);
				break;
			case AniTree::TYPE_FLOAT:
				return CheckData(triggerType, *reinterpret_cast<float*>(targetVariable), data_f);
				break;
			case AniTree::TYPE_INT:
				return CheckData(triggerType, *reinterpret_cast<int*>(targetVariable), data_i);
				break;
			case AniTree::TYPE_UINT:
				return CheckData(triggerType, *reinterpret_cast<unsigned int*>(targetVariable), data_u);
				break;
			}

			return false;
		}
	};

	enum TO_ANI_NODE_TYPE
	{
		TO_ANI_NODE_TYPE_ONE_OK,
		TO_ANI_NODE_TYPE_ALL_OK,
	};

	struct ToAniNode
	{
		ToAniNode(unsigned int toNodeIndex)
			:targetNodeIndex(toNodeIndex)
		{

		}

		bool IsTriggersReadyOK(unsigned long long currTick, unsigned int clipEnd)
		{
			size_t triggerIndex = 0;

			switch (type)
			{
			case AniTree::TO_ANI_NODE_TYPE_ONE_OK:
			{
				for (auto& it : changeConditions)
				{
					if (it == CHANGE_CONDITION_TYPE_ANI_END)
					{
						if (currTick >= clipEnd)
						{
							return true;
						}
					}
					else
					{
						if (triggerDatas[triggerIndex++].IsTriggerOK())
						{
							return true;
						}
					}
				}

				return false;
			}
			break;
			case AniTree::TO_ANI_NODE_TYPE_ALL_OK:
			{
				for (auto& it : changeConditions)
				{
					if (it == CHANGE_CONDITION_TYPE_ANI_END)
					{
						if (currTick < clipEnd)
						{
							return false;
						}
					}
					else
					{
						if (!triggerDatas[triggerIndex++].IsTriggerOK())
						{
							return false;
						}
					}
				}

				return true;
			}
			break;
			default:
			{
				assert(false);
				return false;
			}
			break;
			}
		}

		TO_ANI_NODE_TYPE type;
		const unsigned int targetNodeIndex;
		std::vector<CHANGE_CONDITION_TYPE> changeConditions;
		std::vector<TriggerData> triggerDatas;
	};

	class AniNode
	{
	public:
		AniNode(std::string& ani, unsigned int aniClipEndTime, unsigned int uIndex)
			: targetAniName(ani)
			, index(uIndex)
			, aniEndTime(aniClipEndTime)
			, currTick(0)
		{

		}

		int Update(unsigned int deltaTime);

	private:
		const unsigned int index;
		const unsigned int aniEndTime;
		const std::string targetAniName;

		unsigned long long currTick;
		std::vector<ToAniNode> toAnis;
	};
}

class AnimationTree
{
public:

private:

};