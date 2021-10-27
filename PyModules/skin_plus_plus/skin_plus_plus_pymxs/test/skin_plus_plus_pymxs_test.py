import site
site.addsitedir(r"D:\Code\Git\SkinPlusPlus\PyModules\skin_plus_plus\x64\Release")

import SkinPlusPlusPymxs

print(SkinPlusPlusPymxs)

weights = SkinPlusPlusPymxs.get_skin_weights("Sphere001")
print(type(weights))
print(len(weights[0]))

print(weights[0][0])