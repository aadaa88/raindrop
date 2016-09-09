function getMoveFunction(sX, sY, eX, eY, obj)
	return function(frac)
		obj.X = sX + (eX - sX)*frac 
		obj.Y = sY + (eY - sY)*frac
		return 1
	end
end

function getUncropFunction(w, h, iw, ih, obj)
	return function(frac)
		obj.Width = w 
		obj.Height = h*(1-frac)
		obj:SetCropByPixels(0, iw, ih*frac*0.5, (ih - (ih*frac*0.5)))
		return 1
	end
end

function getFadeFunction(sF, eF, obj)
	return function (frac)
		obj.Alpha = sF + (eF - sF)*frac
		return 1
	end
end

Ease = {
  ElasticSquare = function(p)
    local attn = 1 + 1 - math.asin(1.0 / p) * 2.0 / math.pi
    return function (x)
      return math.sin(x*x * math.pi / 2.0 * attn) * p
    end
  end
}