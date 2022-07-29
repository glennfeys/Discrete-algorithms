#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stack>
#include <limits>
#include <cmath>
#include <random>
#include <algorithm>
#include <chrono>
#include <vector.hpp>

//#define GIVE_ME_DEBUG_OUTPUT

#define EPSILON (1.0e-6)

std::random_device device;
std::mt19937 generator(device());

struct Disk {
    Vector centroid;
    double radius;

    Disk(Vector centroid, double radius) : centroid(centroid), radius(radius) {}
};

/**
 * Om de geörienteerde oppervlakte aan te vullen met een radius voor de lijn "te verschuiven",
 * moeten we de schalingsfactor berekenen waarmee we de radius vermenigvuldigen. Het idee is dat
 * de driehoek vergroot wordt in de hoogte met radius, dus we moeten nog vermenigvuldigen met de breedte hier.
 * Note dat we overal werken met de dubbele oppervlakte, vandaar dat er geen /2 is zoals normaal bij de oppervlakte
 * van een driehoek.
 * @param {Vector} p Het startpunt van de geörienteerde lijn.
 * @param {Vector} p Het eindpunt van de geörienteerde lijn.
 * @return {double} De schalingsfactor zoals beschreven.
 */
inline double areaFactorForRadius(Vector p, Vector q) {
    Vector line = q - p;
    return line.length();
}

struct Extrema {
    const Disk* dp;
    const Disk* dq;
    Vector p;
    Vector q;

    Extrema swapped() const {
        return {
            .dp = dq,
            .dq = dp,
            .p = q,
            .q = p,
        };
    }
};

Extrema findExtrema(const std::vector<const Disk*>& input) {
    auto dp = input[0];
    auto dq = input[0];
    Vector p = dp->centroid - Vector{ dp->radius, 0.0f };
    Vector q = dq->centroid + Vector{ dq->radius, 0.0f };

    for(auto it = std::begin(input) + 1, end = std::end(input); it != end; ++it) {
        auto disk = *it;

        double candidateX;

        // Geef eerst voorrang naar de x as, en daarna aan de y as.
        candidateX = disk->centroid.x - disk->radius;
        if(candidateX < p.x) {
            dp = disk;
            p = Vector{ candidateX, dp->centroid.y };
        } else if(candidateX == p.x) {
            if(disk->centroid.y - disk->radius > dp->centroid.y - dp->radius) {
                dp = disk;
                p = Vector{ candidateX, dp->centroid.y };
            }
        }

        candidateX = disk->centroid.x + disk->radius;
        if(candidateX > q.x) {
            dq = disk;
            q = Vector{ candidateX, dq->centroid.y };
        } else if(candidateX == q.x) {
            if(disk->centroid.y + disk->radius < dq->centroid.y + dq->radius) {
                dq = disk;
                q = Vector{ candidateX, dq->centroid.y };
            }
        }
    }

    return {
        .dp = dp,
        .dq = dq,
        .p = p,
        .q = q,
    };
}

void pivotDisksInitiallyInLPlusAndRPlus(const std::vector<const Disk*>& input, std::vector<const Disk*>& LPlus, std::vector<const Disk*>& RPlus, Vector p, Vector q) {
    double areaFactor = areaFactorForRadius(p, q);

    for(const auto disk : input) {
        double orient = orientation(p, q, disk->centroid);
        double scaledRadius = disk->radius * areaFactor;

        // Je verschuift zogezegd de lijn zodat de straal bevat blijft.
        // Een disk kan in beide verzamelingen zitten als die crossing is.
        if(orient <= scaledRadius) {
            LPlus.push_back(disk);
        }
        if(orient >= -scaledRadius) {
            RPlus.push_back(disk);
        }
    }
}

void pivotDisksExpandedNegative(const std::vector<const Disk*>& input, std::vector<const Disk*>& output, Extrema extrema, double e, bool alwaysAddExtremaDisks) {
    double areaFactor = areaFactorForRadius(extrema.p, extrema.q);

    for(auto disk : input) {
        // Als we ze altijd toevoegen gaan we hier niets doen ermee om dubbels te vermijden.
        if(alwaysAddExtremaDisks && (disk == extrema.dp || disk == extrema.dq))
            continue;

        double orient = orientation(extrema.p, extrema.q, disk->centroid);
        double scaledRadius = disk->radius * areaFactor;

        // Je verschuift zogezegd de lijn zodat de straal bevat blijft.
        if(orient <= scaledRadius - e) {
            output.push_back(disk);
        }
    }

    if(alwaysAddExtremaDisks) {
        output.push_back(extrema.dp);
        // In principe is het mogelijk dat de extrema samenvallen, en we willen geen dubbels.
        if(extrema.dp != extrema.dq) output.push_back(extrema.dq);
    }
}

void pivotDisksOnPositiveNonPositive(const std::vector<const Disk*>& input, std::vector<const Disk*>& onPositive, std::vector<const Disk*>& nonPositive, Vector p, Vector q, const Disk* dp, const Disk* dq) {
    double areaFactor = areaFactorForRadius(p, q);

    for(const auto& disk : input) {
        double orient = orientation(p, q, disk->centroid);
        double scaledRadius = disk->radius * areaFactor;

        // Je verschuift zogezegd de lijn zodat de straal bevat blijft.
        if(fabs(orient - scaledRadius) < EPSILON && disk != dp && disk != dq) {
            onPositive.push_back(disk);
        } else if(orient < scaledRadius + EPSILON) {
            nonPositive.push_back(disk);
        }
    }
}

std::pair<const Disk*, Vector> findApex(std::vector<const Disk*>& input, Extrema extrema) {
    Vector line = extrema.q - extrema.p;
    double areaFactor = areaFactorForRadius(extrema.p, extrema.q);

    // Zoek apex disk.
    // Het punt dat zorgt voor de grootste oppervlakte van de driehoek, is diegene dat het verste verwijderd zit.
    // Voor eenzelfde basis is de oppervlakte met hoogte 1 + hoogte 2 dezelfde als 1 driehoek oppervlakte met de som van de hoogteN.
    // Dus we kunnen de straal * basis optellen voor een driehoek tot het uiterste punt te krijgen.
    std::vector<const Disk*> apexCandidates;
    double area = -std::numeric_limits<double>::infinity();
    for(auto disk : input) {
        double currentArea = disk->radius * areaFactor - orientation(extrema.p, extrema.q, disk->centroid);

#ifdef GIVE_ME_DEBUG_OUTPUT
        std::cout << "  " << disk->centroid << ": " << currentArea << std::endl;
#endif

        // Als er meerdere mogelijke apexes zijn moeten we random een kiezen.
        if(currentArea > area - EPSILON) {
            area = currentArea;
            apexCandidates.clear();
            apexCandidates.push_back(disk);
        } else if(fabs(currentArea - area) < EPSILON) {
            apexCandidates.push_back(disk);
        }
    }

    //std::cout << apexCandidates.size() << " " << input.size() << std::endl;
    std::uniform_int_distribution<size_t> distribution(0, apexCandidates.size() - 1);
    auto disk = apexCandidates[distribution(generator)];

    //std::cout << "(apex)  disk: " << disk->centroid << std::endl;
    //std::cout << "(apex)  line: " << extrema.p << " to " << extrema.q << " => " << line << std::endl;

    Vector normal { line.y, -line.x };
    normal = normal / normal.length();

    Vector point = disk->centroid + normal * disk->radius;
    //std::cout << "(apex)  point: " << point << std::endl;

    return { disk, point };
}

std::pair<Vector, Vector> calculateCommonTangent(const Disk& d1, const Disk& d2) {
#ifdef GIVE_ME_DEBUG_OUTPUT
    std::cout << "Calculate common tangent for: " << d1.centroid << ", " << d2.centroid << std::endl;
#endif

    Vector difference = d2.centroid - d1.centroid;
    double distanceBetweenCentroids = difference.length();

    double angleFromXAxis = atan2f(difference.y, difference.x);
    double angleFromNormal = acosf((d1.radius - d2.radius) / distanceBetweenCentroids);

    double totalAngle = angleFromXAxis + angleFromNormal; // [-pi, 2pi]

    totalAngle += M_PI * 2.0;
    totalAngle = fmodf(totalAngle, M_PI * 2.0); // [0, 2pi)
    // Zorg dat altijd de disks zich bevinden aan de positieve kant.
    if(totalAngle <= M_PI) {
        //Vector startPointOfTangent2 = d1.centroid + Vector{ totalAngle } * d1.radius;
        //Vector endPointOfTangent2 = d2.centroid + Vector{ totalAngle } * d2.radius;
        //std::cout << "Would've been: " << startPointOfTangent2 << ", " << endPointOfTangent2 << std::endl;
        totalAngle = angleFromXAxis - angleFromNormal;
    }

    Vector offset { totalAngle };

    Vector startPointOfTangent = d1.centroid + offset * d1.radius;
    Vector endPointOfTangent = d2.centroid + offset * d2.radius;

    return { startPointOfTangent, endPointOfTangent };
}

void dumpFrontAndBack(const std::vector<const Disk*>& front, const std::vector<const Disk*>& back) {
    std::cout << "F: " << front.size() << ", B: " << back.size() << std::endl;
    for(const auto f : front) {
        std::cout << f->centroid << std::endl;
    }
    for(const auto b : back) {
        std::cout << b->centroid << std::endl;
    }
}

struct WorkListItem {
    std::vector<const Disk*> input;
    Extrema extrema;

    WorkListItem(std::vector<const Disk*>&& input, Extrema extrema) : input(input), extrema(extrema) {}
};

void findHull(std::stack<WorkListItem> workList, std::vector<const Disk*>& output) {
    while(!workList.empty()) {
        auto item = workList.top();
        auto input = std::move(item.input);
        auto extrema = std::move(item.extrema);
        workList.pop();

#ifdef GIVE_ME_DEBUG_OUTPUT
        std::cout << "findHull: " << extrema.p << ", " << extrema.q << " " << extrema.dp->centroid << " " << extrema.dq->centroid << input.size() << std::endl;
#endif

        // Basisgevallen van de recursie.
        if(input.size() <= 1) {
            output.push_back(extrema.dp);
            continue;
        }
        if(input.size() == 2 && extrema.dp != extrema.dq) {
            output.push_back(extrema.dp);
            output.push_back(extrema.dq);
            continue;
        }

        auto [ apex, apexPt ] = findApex(input, extrema);
#ifdef GIVE_ME_DEBUG_OUTPUT
        std::cout << "Apex " << apexPt << std::endl;
#endif

        /* 
         * 
         *      p------q
         *       \    /
         *  F ->  \  / <- B
         *         \/
         *        apex
         */

        Extrema frontExtrema { .dp = extrema.dp, .dq = apex, .p = extrema.p, .q = apexPt };
        Extrema backExtrema { .dp = apex, .dq = extrema.dq, .p = apexPt, .q = extrema.q };

        std::vector<const Disk*> front, back;
        pivotDisksExpandedNegative(input, front, frontExtrema, EPSILON, true);
        pivotDisksExpandedNegative(input, back, backExtrema, EPSILON, true);

        //dumpFrontAndBack(front, back);

        // Check de sliver case.
        if((front.size() == input.size() && back.size() == 1) || (back.size() == input.size() && front.size() == 1)) {
            front.clear();
            back.clear();

            // Bereken eerst de niet-negatieve raaklijn tussen de extrema.
            auto [ startPt, endPt ] = calculateCommonTangent(*extrema.dp, *extrema.dq);
#ifdef GIVE_ME_DEBUG_OUTPUT
            std::cout << "sliver line: " << startPt << ", " << endPt << std::endl;
#endif

            std::vector<const Disk*> onPositive, nonPositive;
            pivotDisksOnPositiveNonPositive(input, onPositive, nonPositive, startPt, endPt, extrema.dp, extrema.dq);

            Extrema apexFindingExtrema = { .dp = extrema.dp, .dq = extrema.dq, .p = startPt, .q = endPt };

            if(nonPositive.size() == 0 && onPositive.size() == 0 /* zal 0 zijn als enkel onze eigen extrema er zouden zitten moesten we niet uitsluiten */) {
#ifdef GIVE_ME_DEBUG_OUTPUT
                std::cout << "case A" << std::endl;
#endif

                apex = extrema.dp;
                apexPt = startPt;
            } else if(nonPositive.size() == 0 && onPositive.size() >= 1) {
#ifdef GIVE_ME_DEBUG_OUTPUT
                std::cout << "case B" << std::endl;
#endif

                auto [ newApex, newApexPt ] = findApex(onPositive, apexFindingExtrema);
                apex = newApex;
                apexPt = newApexPt;
            } else /* if(nonPositive.size() >= 1) */ {
#ifdef GIVE_ME_DEBUG_OUTPUT
                std::cout << "case C1 & C2" << std::endl;
#endif

                auto [ newApex, newApexPt ] = findApex(nonPositive, apexFindingExtrema);
                apex = newApex;
                apexPt = newApexPt;
            }

            // Do it again with the new apex.
            frontExtrema = { .dp = extrema.dp, .dq = apex, .p = extrema.p, .q = apexPt };
            backExtrema = { .dp = apex, .dq = extrema.dq, .p = apexPt, .q = extrema.q };

            //std::cout << "(sliver) extrema: " << extrema.p << ", " << extrema.q << std::endl;
            //std::cout << "(sliver) front: " << frontExtrema.p << ", " << frontExtrema.q << std::endl;
            //std::cout << "(sliver) back: " << backExtrema.p << ", " << backExtrema.q << std::endl;

            pivotDisksExpandedNegative(input, front, frontExtrema, EPSILON, true);
            pivotDisksExpandedNegative(input, back, backExtrema, EPSILON, true);
            
            //dumpFrontAndBack(front, back);
        }

        workList.emplace(std::move(front), frontExtrema);
        workList.emplace(std::move(back), backExtrema);
    }
}

void quickhullDisk(std::vector<const Disk*>& input, std::vector<const Disk*>& output) {
    if(input.size() == 0) return;
    if(input.size() == 1) { output.push_back(input[0]); return; }

    auto extrema = findExtrema(input);

    std::vector<const Disk*> LPlus, RPlus;
    pivotDisksInitiallyInLPlusAndRPlus(input, LPlus, RPlus, extrema.p, extrema.q);

    std::stack<WorkListItem> workList;
    workList.emplace(std::move(LPlus), extrema);
    workList.emplace(std::move(RPlus), extrema.swapped());
    findHull(std::move(workList), output);
}

void parseFile(const char* fileName, std::vector<const Disk*>& output) {
    // C-style om performance redenen.
    int uselessId;
    double x, y, r;

    auto fp = fopen(fileName, "r");
    if(!fp) {
        throw std::runtime_error("Could not open source file");
    }
    char buf[80];
    fgets(buf, sizeof(buf), fp); // Skip eerste lijn, die aantal disks zegt, maar dat maakt voor ons toch niet uit.
    while(fscanf(fp, "%d\t%lg\t%lg\t%lg", &uselessId, &x, &y, &r) != EOF) {
        output.push_back(new Disk{ { x, y }, r });
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    std::vector<const Disk*> allDisks;
    try {
        parseFile(argv[1], allDisks);
    } catch(std::exception& e) {
        std::cerr << "Parsing failure: " << e.what() << std::endl;
        return 2;
    }

    // Het is sneller om met een vector te werken en achteraf de dubbels te wissen, dan met een unordered_set.
    auto before = std::chrono::high_resolution_clock::now();
    std::vector<const Disk*> output;
    quickhullDisk(allDisks, output);

    // Dedup
    std::sort(std::begin(output), std::end(output));
    output.erase(std::unique(std::begin(output), std::end(output)), std::end(output));

    auto after = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(after - before);

    for(const auto disk : output) {
        std::cout << disk->centroid << std::endl;
    }
    std::cout << "Took: " << microseconds.count() << "µs" << std::endl;

    for(const auto disk : allDisks) {
        delete disk;
    }

    return 0;
}
