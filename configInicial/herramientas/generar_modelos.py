"""
Genera los modelos 3D del proyecto en formato OBJ (con su .mtl y textura).
Cada modelo se arma con figuras simples (cajas y cilindros), igual que se haría
en Blender, y se exporta a la carpeta Models para cargarlo con Assimp.

Uso:  python generar_modelos.py   (desde la carpeta herramientas)
"""
import math, os
from PIL import Image, ImageDraw

SALIDA = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Models')

class Obj:
    def __init__(self): self.v, self.vt, self.vn, self.f = [], [], [], []
    def _vert(self, p):  self.v.append(p); return len(self.v)
    def _uv(self, t):    self.vt.append(t); return len(self.vt)
    def _n(self, n):     self.vn.append(n); return len(self.vn)
    def quad(self, pts, n, uv):
        ni = self._n(n); ti = self._uv(uv)
        ids = [self._vert(p) for p in pts]
        self.f.append([(i, ti, ni) for i in ids])
    def caja(self, c, s, uv, rot=None):
        """Caja con centro c, tamaño s; rot = (eje, grados) opcional."""
        hx, hy, hz = s[0]/2, s[1]/2, s[2]/2
        caras = [((0,0,1),  [(-hx,-hy,hz),(hx,-hy,hz),(hx,hy,hz),(-hx,hy,hz)]),
                 ((0,0,-1), [(hx,-hy,-hz),(-hx,-hy,-hz),(-hx,hy,-hz),(hx,hy,-hz)]),
                 ((-1,0,0), [(-hx,-hy,-hz),(-hx,-hy,hz),(-hx,hy,hz),(-hx,hy,-hz)]),
                 ((1,0,0),  [(hx,-hy,hz),(hx,-hy,-hz),(hx,hy,-hz),(hx,hy,hz)]),
                 ((0,1,0),  [(-hx,hy,hz),(hx,hy,hz),(hx,hy,-hz),(-hx,hy,-hz)]),
                 ((0,-1,0), [(-hx,-hy,-hz),(hx,-hy,-hz),(hx,-hy,hz),(-hx,-hy,hz)])]
        for n, pts in caras:
            if rot:
                n = rotar(n, rot); pts = [rotar(p, rot) for p in pts]
            self.quad([(p[0]+c[0], p[1]+c[1], p[2]+c[2]) for p in pts], n, uv)
    def cilindro(self, c, r, h, uv_lado, uv_tapa, seg=20):
        """Cilindro vertical con la base en c."""
        for i in range(seg):
            a0, a1 = 2*math.pi*i/seg, 2*math.pi*(i+1)/seg
            p0 = (c[0]+r*math.cos(a0), c[2]+r*math.sin(a0)); p1 = (c[0]+r*math.cos(a1), c[2]+r*math.sin(a1))
            am = (a0+a1)/2
            self.quad([(p0[0],c[1],p0[1]), (p1[0],c[1],p1[1]), (p1[0],c[1]+h,p1[1]), (p0[0],c[1]+h,p0[1])],
                      (math.cos(am),0,math.sin(am)), uv_lado)
            # tapas (triángulos en abanico, guardados como cuadriláteros degenerados)
            self.quad([(c[0],c[1]+h,c[2]), (p1[0],c[1]+h,p1[1]), (p0[0],c[1]+h,p0[1]), (c[0],c[1]+h,c[2])], (0,1,0), uv_tapa)
            self.quad([(c[0],c[1],c[2]), (p0[0],c[1],p0[1]), (p1[0],c[1],p1[1]), (c[0],c[1],c[2])], (0,-1,0), uv_tapa)
    def guardar(self, nombre, textura):
        with open(os.path.join(SALIDA, nombre + '.mtl'), 'w') as m:
            m.write(f'newmtl {nombre}\nKa 1 1 1\nKd 1 1 1\nKs 0.2 0.2 0.2\nNs 16\nmap_Kd {textura}\n')
        with open(os.path.join(SALIDA, nombre + '.obj'), 'w') as o:
            o.write(f'# {nombre} - Proyecto Arena de Hockey (generado con generar_modelos.py)\n')
            o.write(f'mtllib {nombre}.mtl\no {nombre}\n')
            for p in self.v:  o.write('v %.4f %.4f %.4f\n' % p)
            for t in self.vt: o.write('vt %.4f %.4f\n' % t)
            for n in self.vn: o.write('vn %.4f %.4f %.4f\n' % n)
            o.write(f'usemtl {nombre}\n')
            for f in self.f:  # cada cuadrilátero se guarda como dos triángulos
                a, b, c, d = f
                for tri in ((a, b, c), (a, c, d)):
                    o.write('f ' + ' '.join(f'{i}/{t}/{n}' for i, t, n in tri) + '\n')
        print(nombre, len(self.v), 'vértices')

def rotar(p, rot):
    eje, g = rot; a = math.radians(g); c, s = math.cos(a), math.sin(a); x, y, z = p
    if eje == 'x': return (x, y*c - z*s, y*s + z*c)
    if eje == 'z': return (x*c - y*s, x*s + y*c, z)
    return (x*c + z*s, y, -x*s + z*c)

# Atlas de colores: imagen de 4x4 celdas; cada parte del modelo toma el color de una celda
def atlas(nombre, colores):
    img = Image.new('RGBA', (64, 64), (255, 0, 255, 255)); d = ImageDraw.Draw(img)
    for (cx, cy), col in colores.items():
        d.rectangle([cx*16, cy*16, cx*16+15, cy*16+15], fill=col)
    img.save(os.path.join(SALIDA, nombre))
def celda(cx, cy): return ((cx + 0.5) / 4, 1 - (cy + 0.5) / 4)

PIEL, PELO, PLAYERA, PANTALON, TENIS, FRANJA = celda(0,0), celda(1,0), celda(2,0), celda(3,0), celda(0,1), celda(1,1)

# ---------- Aficionado sentado (mira hacia +z, origen en el piso bajo el asiento) ----------
def aficionado(nombre, color_playera, color_franja):
    o = Obj()
    for lado in (-1, 1):
        x = 0.1 * lado
        o.caja((x, 0.05, 0.38), (0.12, 0.1, 0.22), TENIS)        # tenis
        o.caja((x, 0.30, 0.35), (0.12, 0.42, 0.12), PANTALON)    # espinilla
        o.caja((x, 0.51, 0.17), (0.14, 0.13, 0.42), PANTALON)    # muslo
        o.caja((0.23 * lado, 0.86, 0.04), (0.11, 0.38, 0.12), PLAYERA)                     # brazo
        o.caja((0.2 * lado, 0.66, 0.2), (0.1, 0.1, 0.3), PIEL)                              # antebrazo
    o.caja((0, 0.86, 0.04), (0.38, 0.6, 0.24), PLAYERA)          # torso
    o.caja((0, 0.78, 0.04), (0.385, 0.08, 0.245), FRANJA)        # franja de la playera
    o.caja((0, 1.21, 0.04), (0.2, 0.24, 0.22), PIEL)             # cabeza
    o.caja((0, 1.34, 0.02), (0.22, 0.06, 0.24), PELO)            # pelo
    o.guardar(nombre, nombre + '.png')
    atlas(nombre + '.png', {(0,0): (224,178,140,255), (1,0): (45,30,20,255), (2,0): color_playera,
                            (3,0): (40,42,55,255), (0,1): (235,235,235,255), (1,1): color_franja})

# ---------- Palo de hockey (origen en el talón de la pala, mango hacia +y) ----------
def palo():
    o = Obj()
    o.caja((0, 0.80, 0), (0.032, 1.5, 0.025), celda(0,0))        # mango
    o.caja((0, 1.5, 0), (0.036, 0.12, 0.029), celda(1,0))        # cinta del puño
    o.caja((0.15, 0.04, 0), (0.32, 0.075, 0.012), celda(2,0), rot=('y', 0))  # pala con cinta negra
    o.guardar('palo', 'palo.png')
    atlas('palo.png', {(0,0): (190,150,90,255), (1,0): (240,240,240,255), (2,0): (25,25,25,255)})

# ---------- Botella de agua ----------
def botella():
    o = Obj()
    o.cilindro((0, 0, 0), 0.035, 0.2, celda(0,0), celda(1,0))
    o.cilindro((0, 0.2, 0), 0.015, 0.04, celda(2,0), celda(2,0), seg=12)
    o.guardar('botella', 'botella.png')
    atlas('botella.png', {(0,0): (40,140,70,255), (1,0): (60,170,90,255), (2,0): (240,240,240,255)})

# ---------- Disco (puck): 3 in de diámetro, 1 in de alto ----------
def disco():
    o = Obj()
    o.cilindro((0, 0, 0), 0.0381, 0.0254, celda(0,0), celda(1,0), seg=24)
    o.guardar('disco', 'disco.png')
    atlas('disco.png', {(0,0): (20,20,20,255), (1,0): (35,35,35,255)})

if __name__ == '__main__':
    os.makedirs(SALIDA, exist_ok=True)
    aficionado('aficionado_local', (200,30,40,255), (245,245,245,255))
    aficionado('aficionado_visita', (25,70,170,255), (240,200,30,255))
    palo(); botella(); disco()
