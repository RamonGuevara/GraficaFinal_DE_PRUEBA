"""
Genera las texturas del proyecto en la carpeta Textures.
La pista se dibuja a escala con las medidas del reglamento NHL (1 ft = 0.3048 m).
Uso:  python generar_texturas.py   (desde la carpeta herramientas)
"""
from PIL import Image, ImageDraw, ImageFont
import random, math, os
T = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Textures')
def ruta(n): return os.path.join(T, n)
FUENTE = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
FT = 0.3048
PX = 40                      # pixeles por metro
L, A, R = 200*FT, 85*FT, 28*FT
W, H = int(L*PX), int(A*PX)
S = 4                        # supermuestreo para bordes suaves
def p(x_ft, y_ft):           # coordenadas en pies desde el centro -> pixeles (supermuestreo)
    return ((x_ft*FT + L/2)*PX*S, (y_ft*FT + A/2)*PX*S)
def r(ft): return ft*FT*PX*S

big = Image.new('RGBA', (W*S, H*S), (0,0,0,0))
d = ImageDraw.Draw(big)
# hielo con esquinas redondeadas
d.rounded_rectangle([0,0,W*S-1,H*S-1], radius=r(28), fill=(236,242,246,255))
RED, BLUE, LBLUE = (200,30,40,255), (20,60,170,255), (120,180,230,255)
def vline(x, w_ft, col):
    d.rectangle([p(x - w_ft/2, -42.5)[0], 0, p(x + w_ft/2, 42.5)[0], H*S], fill=col)
def circle(x, y, rad, col, w_ft=2/12, fill=None):
    cx, cy = p(x, y); d.ellipse([cx-r(rad), cy-r(rad), cx+r(rad), cy+r(rad)], outline=col, width=max(1,int(r(w_ft))), fill=fill)
def dot(x, y, rad, col):
    cx, cy = p(x, y); d.ellipse([cx-r(rad), cy-r(rad), cx+r(rad), cy+r(rad)], fill=col)
# áreas de portero (semicirculo de 6 ft frente a la linea de gol)
for s in (-1, 1):
    gx = s*(100-11)
    cx, cy = p(gx, 0)
    box = [cx-r(6), cy-r(6), cx+r(6), cy+r(6)]
    start, end = (90, 270) if s > 0 else (-90, 90)
    d.pieslice(box, start, end, fill=LBLUE, outline=RED, width=int(r(2/12)))
# lineas de gol (2 in), solo dentro de la curva
for s in (-1, 1):
    gx = s*(100-11)
    yy = math.sqrt(28**2 - (28-11)**2)            # interseccion con la esquina
    half = 42.5 - 28 + yy
    d.rectangle([p(gx-1/12, -half)[0], p(0, -half)[1], p(gx+1/12, half)[0], p(0, half)[1]], fill=RED)
    # trapecio detras de la porteria
    for t in (-1, 1):
        d.line([p(gx, t*11), p(s*100, t*14)], fill=RED, width=int(r(2/12)))
# lineas azules (12 in) a 75 ft de cada fondo, linea roja central (12 in)
for s in (-1, 1): vline(s*(100-75.5), 1, BLUE)
vline(0, 1, RED)
# circulo central (azul) y punto central
circle(0, 0, 15, BLUE); dot(0, 0, 0.5, BLUE)
# circulos de saque en zonas de ataque (20 ft desde linea de gol, 22 ft a los lados)
for s in (-1, 1):
    for t in (-1, 1):
        x, y = s*(100-11-20), t*22
        circle(x, y, 15, RED); dot(x, y, 1, RED)
        for u in (-1, 1):          # marcas de hash en el circulo
            for v in (-1, 1):
                hx = x + u*(2+5/12/2)
                d.line([p(hx, y+v*14.9), p(hx, y+v*17)], fill=RED, width=int(r(2/12)))
# puntos de saque de la zona neutral (5 ft de la linea azul)
for s in (-1, 1):
    for t in (-1, 1): dot(s*(100-75-1-5), t*22, 1, RED)
# semicirculo del arbitro (10 ft) junto a la mesa de oficiales
cx, cy = p(0, 42.5); d.arc([cx-r(10), cy-r(10), cx+r(10), cy+r(10)], 180, 360, fill=RED, width=int(r(2/12)))
img = big.resize((W, H), Image.LANCZOS)
# ruido suave para que el hielo no se vea plano
px = img.load(); random.seed(3)
for y in range(H):
    for x in range(W):
        rr, gg, bb, aa = px[x, y]
        if aa:
            n = random.randint(-6, 6)
            px[x, y] = (max(0,min(255,rr+n)), max(0,min(255,gg+n)), max(0,min(255,bb+n)), aa)
img.save(ruta('pista_hielo.png'))
print('pista', img.size)

# vallas con publicidad (anuncios inventados): 4 tableros, la textura cubre 16 m de valla
v = Image.new('RGBA', (2048, 128), (245,245,245,255)); dv = ImageDraw.Draw(v)
dv.rectangle([0, 104, 2048, 128], fill=(240,200,20,255))   # franja amarilla de abajo
dv.rectangle([0, 0, 2048, 6], fill=(20,60,170,255))        # línea azul de arriba
anuncios = [('FI · UNAM', (0,60,130), (255,255,255)), ('COMPUTACIÓN GRÁFICA', (255,255,255), (200,30,40)),
            ('HIELO MX', (200,30,40), (255,255,255)), ('OPENGL 3.3', (255,255,255), (20,60,170))]
f = ImageFont.truetype(FUENTE, 44)
for i, (txt, fondo, letra) in enumerate(anuncios):
    x0 = i*512 + 40
    dv.rectangle([x0, 22, x0+432, 92], fill=fondo)
    tam = 44
    while ImageFont.truetype(FUENTE, tam).getlength(txt) > 400: tam -= 2   # que quepa en el tablero
    fa = ImageFont.truetype(FUENTE, tam)
    w = dv.textlength(txt, font=fa)
    dv.text((x0 + 216 - w/2, 57 - tam*0.6), txt, font=fa, fill=letra)
v.save(ruta('valla.png'))

# pantalla del marcador (los números se dibujan aparte con la textura de dígitos)
m = Image.new('RGBA', (512, 256), (12,12,16,255)); dm = ImageDraw.Draw(m)
dm.rectangle([4,4,507,251], outline=(90,90,100,255), width=6)
f2 = ImageFont.truetype(FUENTE, 30)
for txt, cx, col in (('LOCAL', 100, (230,60,60)), ('PERIODO', 256, (220,220,220)), ('VISITA', 412, (80,130,240))):
    w = dm.textlength(txt, font=f2); dm.text((cx - w/2, 22), txt, font=f2, fill=col)
for cx in (100, 412): dm.rectangle([cx-80, 80, cx+80, 220], outline=(60,60,70), width=3)
f3 = ImageFont.truetype(FUENTE, 90)
w = dm.textlength('1', font=f3); dm.text((256 - w/2, 90), '1', font=f3, fill=(240,170,40))
m.save(ruta('marcador.png'))

# dígitos 0 a 9 en una tira (el shader elige uno moviendo la coordenada de textura)
dg = Image.new('RGBA', (640, 96), (0,0,0,0)); dd = ImageDraw.Draw(dg)
f4 = ImageFont.truetype(FUENTE, 88)
for i in range(10):
    w = dd.textlength(str(i), font=f4); dd.text((i*64 + 32 - w/2, -6), str(i), font=f4, fill=(255,190,50,255))
dg.save(ruta('digitos.png'))

# red de porteria: rejilla blanca, huecos transparentes (se descartan en el shader)
n = Image.new('RGBA', (128, 128), (0,0,0,0)); dn = ImageDraw.Draw(n)
for i in range(0, 128, 32):
    dn.line([i, 0, i, 128], fill=(250,250,250,255), width=4); dn.line([0, i, 128, i], fill=(250,250,250,255), width=4)
n.save(ruta('red.png'))

# blanco 1x1 para objetos de un solo color (se pinta con el tinte)
Image.new('RGBA', (2, 2), (255,255,255,255)).save(ruta('blanco.png'))
