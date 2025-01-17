#include "PoseHistory.h"
#include "Utils.h"
#include "Logger.h"
#include <mutex>
#include <optional>

void PoseHistory::OnPoseUpdated(const TrackingInfo &info) {
	// Put pose history buffer
	TrackingHistoryFrame history;
	history.info = info;


	HmdMatrix_QuatToMat(info.HeadPose_Pose_Orientation.w,
		info.HeadPose_Pose_Orientation.x,
		info.HeadPose_Pose_Orientation.y,
		info.HeadPose_Pose_Orientation.z,
		&history.rotationMatrix);

	Debug("Rotation Matrix=(%f, %f, %f, %f) (%f, %f, %f, %f) (%f, %f, %f, %f)\n"
		, history.rotationMatrix.m[0][0], history.rotationMatrix.m[0][1], history.rotationMatrix.m[0][2], history.rotationMatrix.m[0][3]
		, history.rotationMatrix.m[1][0], history.rotationMatrix.m[1][1], history.rotationMatrix.m[1][2], history.rotationMatrix.m[1][3]
		, history.rotationMatrix.m[2][0], history.rotationMatrix.m[2][1], history.rotationMatrix.m[2][2], history.rotationMatrix.m[2][3]);

	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_poseBuffer.size() == 0) {
		m_poseBuffer.push_back(history);
	}
	else {
		if (m_poseBuffer.back().info.FrameIndex != info.FrameIndex) {
			// New track info
			m_poseBuffer.push_back(history);
		}
	}
	if (m_poseBuffer.size() > 10) {
		m_poseBuffer.pop_front();
	}
}

std::optional<PoseHistory::TrackingHistoryFrame> PoseHistory::GetBestPoseMatch(const vr::HmdMatrix34_t &pose) const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	float minDiff = 100000;
	int index = 0;
	int minIndex = 0;
	auto minIt = m_poseBuffer.begin();
	for (auto it = m_poseBuffer.begin(); it != m_poseBuffer.end(); it++, index++) {
		float distance = 0;
		// Rotation matrix composes a part of ViewMatrix of TrackingInfo.
		// Be carefull of transpose.
		// And bottom side and right side of matrix should not be compared, because pPose does not contain that part of matrix.
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				distance += pow(it->rotationMatrix.m[j][i] - pose.m[j][i], 2);
			}
		}
		//LogDriver("diff %f %llu", distance, it->info.FrameIndex);
		if (minDiff > distance) {
			minIndex = index;
			minIt = it;
			minDiff = distance;
		}
	}
	if (minIt != m_poseBuffer.end()) {
		return *minIt;
	}
	return {};
}

std::optional<PoseHistory::TrackingHistoryFrame> PoseHistory::GetPoseAt(uint64_t client_timestamp_us) const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	for (const auto& pose: m_poseBuffer)
	{
		if (pose.info.clientTime >= client_timestamp_us)
			return pose;
	}
	return {};
}
