pyTime = python.import "time"


_SKINPPOPS = SkinPPOps()
SkinPP.GetSkinWeights($Sphere002)

fn mxsGetSkinWeights node = (
	skops = SkinOps()
	skopGetVertexWeight = skops.GetVertexWeight
	skopGetVertexWeightCount = skops.GetVertexWeightCount
	skopGetVertexWeightBoneID = skops.GetVertexWeightBoneID

	_polyOp = PolyOp()
	polyOpGetVert = _polyOp.GetVert
	getVertPosition = polyOpGetVert
	if ClassOf node == Editable_mesh do
	(
		_meshOp = MeshOp()
		getVertPosition = _meshOp.GetVert
	)

	skinModifier = node.Modifiers[Skin]
	vertexCount = skops.GetNumberVertices(skinModifier)
	skinWeights = #()
	skinBoneIDs = #()
	positions = #()
	for vertexIndex = 1 to vertexCount do
	(
		influenceCount = skopGetVertexWeightCount skinModifier vertexIndex
		vertexWeights = for influenceIndex = 1 to influenceCount collect skopGetVertexWeight skinModifier vertexIndex influenceIndex
		vertexBoneIDs = for influenceIndex = 1 to influenceCount collect skopGetVertexWeightBoneID skinModifier vertexIndex influenceIndex
		position = getVertPosition node vertexIndex
		Append skinWeights vertexWeights
		Append skinBoneIDs vertexBoneIDs
		Append positions #(position.X, position.Y, position.Z)
	)

	#(skinWeights, skinBoneIDs, positions)
)

fn mxsSetSkinWeights node boneIDs weights = (
	skops = SkinOps()
	skopGetVertexWeight = skops.GetVertexWeight
	skopGetVertexWeightCount = skops.GetVertexWeightCount
	skopGetVertexWeightBoneID = skops.GetVertexWeightBoneID
	skopReplaceVertexWeights = skops.ReplaceVertexWeights
	-- skinOps.ReplaceVertexWeights <Skin> <vertex_integer> \ ( <vertex_bone_integer> | <vertex_bone_array> ) \
 	-- ( <weight_float> | <weight_array> )  [(node:<node> | name:<string>)]
	skinModifier = node.Modifiers[Skin]
	vertexCount = skops.GetNumberVertices(skinModifier)
	for vertexIndex = 1 to vertexCount do
	(
		skopReplaceVertexWeights skinModifier vertexIndex boneIDs[vertexIndex] weights[vertexIndex]
	)
)


fn mxs_GetSkinWeights _obj _loopCount = (
	startTime = pyTime.time()
	for a = 1 to _loopCount do GetSkinWeights _obj
	timeTaken = (pyTime.time() - startTime)
	print("mxs: " + (timeTaken as string))

	timeTaken
)

fn cppfp_GetSkinWeights _obj _loopCount = (
	SkinPPGetSkinWeights = SkinPP.GetSkinWeights
	startTime = pyTime.time()
	for a = 1 to _loopCount do SkinPPGetSkinWeights _obj
	timeTaken = (pyTime.time() - startTime)
	print("c++ function publish: " + (timeTaken as string))

	timeTaken
)

fn cpppm_GetSkinWeights _obj _loopCount = (
	SkinPPOpss = SkinPPOps()
	SkinPPOpssGetSkinWeights = SkinPPOpss.GetSkinWeights
	startTime = pyTime.time()
	for a = 1 to _loopCount do SkinPPOpssGetSkinWeights _obj
	timeTaken = (pyTime.time() - startTime)
	print("c++ primative method: " + (timeTaken as string))

	timeTaken
)

fn cpppm2_GetSkinWeights _obj _loopCount = (
	SkinPPOpss = SkinPPOps()
	SkinPPOpssGetSkinWeights2 = SkinPPOpss.GetSkinWeights2
	startTime = pyTime.time()
	for a = 1 to _loopCount do SkinPPOpssGetSkinWeights2 _obj
	timeTaken = (pyTime.time() - startTime)
	print("c++ primative method2: " + (timeTaken as string))

	timeTaken
)

fn cpppf_GetSkinWeights _obj _loopCount = (
	startTime = pyTime.time()
	for a = 1 to _loopCount do SPPGetSkinWeights _obj
	timeTaken = (pyTime.time() - startTime)
	print("c++ primative function: " + (timeTaken  as string))

	timeTaken
)


fn mxs_SetSkinWeights _obj _loopCount _boneIDs _weights = (
	startTime = pyTime.time()
	for a = 1 to _loopCount do SetSkinWeights _obj _boneIDs _weights
	timeTaken = (pyTime.time() - startTime)
	print("mxs taken: " + (timeTaken as string))

	timeTaken
)

fn cppfp_SetSkinWeights _obj _loopCount _boneIDs _weights = (
	SkinPPSetSkinWeights = SkinPP.SetSkinWeights
	startTime = pyTime.time()
	for a = 1 to _loopCount do SkinPPSetSkinWeights _obj _boneIDs _weights
	timeTaken = (pyTime.time() - startTime)
	print("c++ function publish: " + (timeTaken as string))

	timeTaken
)

fn cpppm_SetSkinWeights _obj _loopCount _boneIDs _weights = (
	SkinPPOpss = SkinPPOps()
	SkinPPOpssSetSkinWeights = SkinPPOpss.SetSkinWeights
	startTime = pyTime.time()
	for a = 1 to _loopCount do SkinPPOpssSetSkinWeights _obj _boneIDs _weights #()
	timeTaken = (pyTime.time() - startTime)
	print("c++ primative method: " + (timeTaken as string))

	timeTaken
)

fn cpppf_SetSkinWeights _obj _loopCount _boneIDs _weights = (
	startTime = pyTime.time()
	for a = 1 to _loopCount do SPPSetSkinWeights _obj _boneIDs _weights #()
	timeTaken = (pyTime.time() - startTime)
	print("c++ primative function: " + (timeTaken as string))

	timeTaken
)

if False do
(
	obj = $Sphere002
	loopCount = 1

	mxsGetSkinWeights obj
	getFunctions = #(
		mxs_GetSkinWeights,
		cppfp_GetSkinWeights,
		cpppm_GetSkinWeights,
		cpppm2_GetSkinWeights,
		cpppf_GetSkinWeights
	)

	fn comparePerformance functs = (
		times = for funct in functs collect (
			funct obj loopCount
		)
		maxTime = amax times
		print(maxTime)

		for t in times collect ( format maxTime / t )
	)
)
-- comparePerformance getFunctions


-- weights = for a = 1 to obj.Verts.Count collect #(0.5, 0.5)
-- ids = for a = 1 to obj.Verts.Count collect #(1, 2)
-- "---"