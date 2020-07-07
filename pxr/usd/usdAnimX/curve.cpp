#include "curve.h"
#include <iostream>
PXR_NAMESPACE_OPEN_SCOPE

UsdAnimXCurve::UsdAnimXCurve() 
{ 
};

UsdAnimXCurve::UsdAnimXCurve(const UsdAnimXCurveDesc &desc)
{
    size_t keyframeIndex = 0;
    for(const auto& keyframe: desc.keyframes) {
        addKeyframe(UsdAnimXKeyframe(keyframe, keyframeIndex));
        keyframeIndex++;
    }
}


bool  UsdAnimXCurve::keyframe(double time, adsk::Keyframe &key) const
{
    size_t numKeys = _keyframes.size();
    if (numKeys == 0)
        return false;

    size_t index = findClosest( time );
    if (index == numKeys)
        return last(key);

    // if found key lies before the requested time, choose next key instead
    if (_keyframes[index].time < time)
    {
        if (index != numKeys - 1)
            index++;
        else
            return last(key);
    }
    return keyframeAtIndex(index, key);
}

bool UsdAnimXCurve::keyframeAtIndex(int index, adsk::Keyframe &key) const
{
    size_t numKeys = _keyframes.size();
    if (index < 0 || index >= _keyframes.size())
        return false;

    key = _keyframes[index];
    /*
    key.tanIn.type = tangentType(key.tanIn.type);
    key.tanIn.x = (seconds)results[0].asDouble();
    key.tanIn.y = (seconds)results[2].asDouble();

    key.tanOut.type = tangentType(curve.outTangentType(index));
    key.tanOut.x = (seconds)results[1].asDouble();
    key.tanOut.y = (seconds)results[3].asDouble();
    
    key.linearInterpolation = false;
    key.quaternionW = 1.0;

    // for auto tangents, calculate the value on the fly based on the previous and the next key
    if (key.tanIn.type == adsk::TangentType::Auto || key.tanOut.type == adsk::TangentType::Auto)
    {			
        bool hasPrev = index > 0;
        bool hasNext = index < (int)numKeys - 1;
        adsk::Keyframe prev, next;
        if (hasPrev)
        {
            prev.time = _keyframes[index - 1].time;
            prev.value = _keyframes[index - 1].value;
        }
        if (hasNext)
        {
            next.time = _keyframes[index + 1].time;
            next.value = _keyframes[index + 1].value;
        }

        adsk::CurveInterpolatorMethod interp = key.curveInterpolationMethod(isWeighted());

        if (key.tanIn.type == adsk::TangentType::Auto)
            autoTangent(true, key, hasPrev ? &prev : nullptr, hasNext ? &next : nullptr, interp, key.tanIn.x, key.tanIn.y);
        
        if (key.tanOut.type == adsk::TangentType::Auto)
            autoTangent(false, key, hasPrev ? &prev : nullptr, hasNext ? &next : nullptr, interp, key.tanOut.x, key.tanOut.y);
    }

    if (key.tanOut.type == adsk::TangentType::Linear &&       
        (tangentType(_keyframes[index + 1].tanIn.type) == adsk::TangentType::Linear))
    {
        key.linearInterpolation = true;             
    }

    if (key.tanIn.type == TangentType::Linear &&
        (tangentType(curve.outTangentType(index - 1, &status)) == TangentType::Linear) &&
            status == MS::kSuccess)
    {
        key.linearInterpolation = true;
    }

    if (isRotation())
    {
        double time = MTime(key.time, MTime::kSeconds).as(MTime::uiUnit());
        auto it = quaternionWcache.find(time);
        if (it != quaternionWcache.end())
        {
            key.quaternionW = it->second;
        }
        else
        {
            // there appears to be a bug when trying to fetch quaternionW of a curve node
            // using getAttr -time at a specific frame. Sometimes, the quaternion sign is
            // flipped, so instead we change the time globally to have the curve evaluate
            // properly
            MString result;
            MString cmd = MString("cmds.currentTime(") + time + ")";
            MGlobal::executePythonCommand(cmd);
            cmd = "cmds.getAttr('" + curve.name() + ".quaternionW')";
            MGlobal::executePythonCommand(cmd, result);
            quaternionWcache[time] = key.quaternionW = result.asDouble();
        }
    }
    */
    return true;
}

bool UsdAnimXCurve::first(adsk::Keyframe &key) const
{
    return keyframeAtIndex(0, key);
}

bool UsdAnimXCurve::last(adsk::Keyframe &key) const
{
    return (keyframeCount() > 0) ?
        keyframeAtIndex(keyframeCount() - 1, key) :
        false;
}

adsk::InfinityType UsdAnimXCurve::preInfinityType() const
{
    return _preInfinityType;
}

adsk::InfinityType UsdAnimXCurve::postInfinityType() const
{
    return _postInfinityType;
}

bool UsdAnimXCurve::isWeighted() const
{
    return _weighted;
}

bool UsdAnimXCurve::isStatic() const
{
    return false;
}

unsigned int UsdAnimXCurve::keyframeCount() const
{
    return _keyframes.size();
}

bool UsdAnimXCurve::isRotation() const
{
    return _rotationInterpolationMethod == 
            adsk::CurveRotationInterpolationMethod::Quaternion ||
        _rotationInterpolationMethod == 
            adsk::CurveRotationInterpolationMethod::Slerp ||
        _rotationInterpolationMethod == 
            adsk::CurveRotationInterpolationMethod::Squad;
}

size_t UsdAnimXCurve::findClosest(double time) const
{
    for(const auto& key: _keyframes) {
        if(time<=key.time) return key.index;
    }
}

void UsdAnimXCurve::addKeyframe(const UsdAnimXKeyframe& key)
{
    _keyframes.push_back(key);
}

void UsdAnimXCurve::addKeyframe(double time, double value)
{
    adsk::Tangent tanIn = { 
        adsk::TangentType::Auto, (adsk::seconds)0, (adsk::seconds)0};
    adsk::Tangent tanOut = { 
        adsk::TangentType::Auto, (adsk::seconds)0, (adsk::seconds)0};

    UsdAnimXKeyframe key;
    key.time = time;                        // Time
    key.value = value;                      // Value
    key.index = (int)_keyframes.size();     // Sequential index of the key      
    key.tanIn = tanIn;                      // In-tangent
    key.tanOut = tanOut;                    // Out-tangent
    key.quaternionW = 1.0;                  // W component of a quaternion
    key.linearInterpolation = false;        // Curve linearly interpolated

    _keyframes.push_back(key);
}

void UsdAnimXCurve::removeKeyframe(double time)
{ 
    std::vector<UsdAnimXKeyframe>::iterator i = _keyframes.begin();
    for(;i<_keyframes.end();++i) {
        if(i->time == time)_keyframes.erase(i);
    }
}

void UsdAnimXCurve::removeKeyframe(size_t index)
{
    if(index >= 0 && index < _keyframes.size())
        _keyframes.erase(_keyframes.begin() + index);
}

double UsdAnimXCurve::evaluate(double time) const
{
    return adsk::evaluateCurve(time, *this);
}

std::set<double> UsdAnimXCurve::computeSamples() const
{
    std::set<double> samples;
    for(const auto& keyframe: _keyframes) {
        samples.insert(keyframe.time);
    }
    return samples;
}

PXR_NAMESPACE_CLOSE_SCOPE