from __future__ import annotations


def get_flat_weights(bone_ids: list[list[int]], weights: list[list[float]], max_influence_count: int = 3):
    vertex_count = len(bone_ids)
    m_weights = [0.0] * vertex_count * max_influence_count
    for vertex_index in range(vertex_count):
        vertex_bone_ids = bone_ids[vertex_index]
        array_index = vertex_index * max_influence_count
        for influence_index in range(len(vertex_bone_ids)):
            bone_id = vertex_bone_ids[influence_index]
            if bone_id == -1:
                continue
            index = array_index + bone_id
            m_weights[index] = weights[vertex_index][influence_index]

    return m_weights

names = ["one", "two", "three"]
new_names = ["FART", "three", "one", "two", "RANDOM"]
bone_ids = [[0, 1, -1], [1, 2, -1], [0, 1, 2]]
weights = [[0.25, 0.75, None], [0.5, 0.5, None], [0.25, 0.25, 0.5]]

m_weights = get_flat_weights(bone_ids, weights)

# print(m_weights)
# print(m_weights == [0.25, 0.75, 0.0, 0.5, 0.5, 0.0, 0.25, 0.25, 0.5])


name_map = {}
for index, name in enumerate(new_names):
    name_map[name] = index

new_bone_ids = []
for name in names:
    new_index = name_map[name]
    new_bone_ids.append(new_index)

# print(new_bone_ids)
def get_sorted_bone_ids(bone_ids: list[list[int]], new_bone_id_order: list[int]):
    for vertex in bone_ids:
        for i, bone_id in enumerate(vertex):
            if bone_id == -1:
                continue

            new_index = new_bone_id_order[bone_id]
            vertex[i] = new_index

    return bone_ids


sorted_bone_ids = get_sorted_bone_ids(bone_ids, new_bone_ids)
# print(f"sorted_bone_ids: {sorted_bone_ids}")

m_weights = get_flat_weights(sorted_bone_ids, weights, max_influence_count=len(new_names))

print(f"m_weights: {m_weights}")
for i in range(3):
    span = 5 * i
    vert = m_weights[span:span+5]
    for j, weight in enumerate(vert):
        print(f"{new_names[j]}: {weight}")
    print("-----")

