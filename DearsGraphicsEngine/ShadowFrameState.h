#pragma once

#include <array>
#include <cstdint>

// Only a completed producer in this frame may publish shadow maps. Availability
// is per light (a point light needs all six faces), never inferred from an SRV.
template <unsigned LightCount>
class ShadowFrameState
{
    static_assert(LightCount > 0 && LightCount <= 32);
public:
    void BeginFrame() { *this = {}; }
    void BeginProduction()
    {
        BeginFrame();
        m_producing = true;
    }
    void RecordTwoDimensional(unsigned light)
    {
        if (m_producing && light < LightCount)
        {
            m_pending2D |= std::uint32_t(1) << light;
            m_sealed = false;
        }
    }
    void RecordPointFace(unsigned light, unsigned face)
    {
        if (m_producing && light < LightCount && face < 6)
        {
            m_pointFaces[light] |= 1u << face;
            m_sealed = false;
        }
    }
    // EndShadowPass unbinds the last DSV before sealing the writes. Publication
    // waits until the enclosing typed Shadow callback has returned successfully.
    void SealWrites() { if (m_producing) m_sealed = true; }
    void EndProduction(bool success)
    {
        m_twoDimensional = m_point = 0;
        if (m_producing && m_sealed && success)
        {
            m_twoDimensional = m_pending2D;
            for (unsigned light = 0; light < LightCount; ++light)
                if (m_pointFaces[light] == 0x3fu)
                    m_point |= std::uint32_t(1) << light;
        }
        m_producing = false;
    }
    std::uint32_t TwoDimensionalMask() const { return m_twoDimensional; }
    std::uint32_t PointMask() const { return m_point; }
    bool IsProducing() const { return m_producing; }

private:
    std::uint32_t m_twoDimensional = 0, m_point = 0, m_pending2D = 0;
    std::array<unsigned, LightCount> m_pointFaces{};
    bool m_producing = false, m_sealed = false;
};
